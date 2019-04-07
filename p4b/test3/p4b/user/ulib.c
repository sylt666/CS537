#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define MAX_THREADS 64


typedef struct address_pair {
	void *aligned;
	void *unaligned;
} adpair;
adpair adlist[MAX_THREADS];
//uint adlist[64][3];

//lock_t userlock;



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

//TODO
int thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2){
	//TODO: aligned them!
	//lock_init(&userlock);
	//lock_acquire(&userlock);
	
	void * allocbase = (void *)malloc(4096 + 4095); //NOTE: allocbase stores the start of the malloc address.
	if (allocbase == NULL)
	    return -1;
	//printf(1, "start address of malloc is: %x.\n", (char*)allocbase);
	void *stack = allocbase + (int)(4096 - ((int) allocbase % 4096));
    //printf(1, "alignment = %d\n", (uint)stack % 4096);

    int thread_create_flag = 0;
    for(int i = 0; i < MAX_THREADS; i++){
    	if(adlist[i].aligned == 0 && adlist[i].unaligned == 0){
    		//printf(1, "i : %d in Thread_create: Found slot in adlist - allocbase: %x, stack: %x\n", i, allocbase, stack);
    		thread_create_flag = 1;
    		adlist[i].aligned = allocbase;
    		adlist[i].unaligned = stack;
    		//adlist[i].inuse = 1;
    		break;
    	}
    }
    // found???
    
    if(thread_create_flag == 0){
    	//printf(1, "Thread_create slot not found\n");
    }
    
	int rc = clone(start_routine, arg1, arg2, stack);
	//printf(1, "Thread_create call clone() and get return code: %d.\n", rc);
	//lock_release(&userlock);
	return rc;

}

int thread_join(){
	void * stack = NULL;
	//lock_acquire(&userlock);
	int pid = join(&stack);
	//printf(1, "Thread_join: joined pid = %d, stack value = %x\n", pid, &stack);
	if(pid == -1) return -1;
	//printf(1, "Thread_join---------------\n");

	
	for(int i = 0; i < MAX_THREADS; i++){
		if(adlist[i].unaligned == stack){
		    //printf(1, "Thread_join: stack value = %x\n", stack);
		    void * freead = adlist[i].aligned;
		    adlist[i].aligned = 0;
		    adlist[i].unaligned = 0;
		    //adlist[i].inuse = 0;
		    free(freead);
		    
		    //lock_release(&userlock);
		    //printf(1, "-----------------here\n");
		    return pid;
		}
	}
	//printf(1, "------------------------------------------------Thread_join failed!\n");
	//lock_release(&userlock);
	return -1;
}

static inline int fetch_and_add(int* variable, int value)
  {
      __asm__ volatile("lock; xaddl %0, %1"
        : "+r" (value), "+m" (*variable) // input+output
        : // No input-only
        : "memory"
      );
      return value;
  }



void lock_init( lock_t *lock){
	lock->ticket = 0;
	lock->turn = 0;
}

void lock_acquire( lock_t *lock){
	int myturn = fetch_and_add(&lock->ticket, 1);
	while (lock->turn != myturn)
		;
}

void lock_release( lock_t *lock){
	lock->turn = lock->turn + 1;
}

