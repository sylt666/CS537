#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

uint memunits[64];
uint ustacks[64];
int tindex = -1;

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int 
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  tindex++;
  //printf(1, "Incremented tindex: %d\n", tindex);
  if (tindex > 63) {
     printf(1, "More than 63 threads were created\n");
     tindex--;
     return -1;
  }

  memunits[tindex] = (uint)malloc(2*4096);

  if (memunits[tindex] < 1) {
    tindex--;
    return-1;
  }

  ustacks[tindex] = memunits[tindex];
  ustacks[tindex] += 4096 - (ustacks[tindex] % 4096);
  //printf(1, "Creating thread. Index: %d, Stack: %p\n", tindex, ustacks[tindex]);
  int pid = clone(start_routine, arg1, arg2, (void*)ustacks[tindex]);

  if (pid == -1) {
    free((void*)ustacks[tindex]);
    tindex--;
  }

  return pid;
} 

int
thread_join()
{

  if (tindex < 0) {
    printf(1, "Ran out of threads.\n");
    return -1;
  }

  //printf(1, "Joining thread. Index: %d, Stack: %p\n", tindex, ustacks[tindex]);
  int pid = join((void**)&ustacks[tindex]);
  free((void*)memunits[tindex]);
  tindex--;
  return pid;
}

static inline int FetchAndAdd(int* variable, int value)
{
  __asm__ volatile("lock; xaddl %0, %1"
  : "+r" (value), "+m" (*variable) // input+output
  : // No input-only
  : "memory"
  );
  return value;
}

void
lock_init(lock_t *lock)
{
  lock->ticket = 0;
  lock->turn = 0;
}

void
lock_acquire(lock_t *lock)
{
  int myturn = FetchAndAdd(&lock->ticket, 1);
  while (lock->turn != myturn); //spin
}

void 
lock_release(lock_t *lock)
{
  lock->turn += 1;
}
