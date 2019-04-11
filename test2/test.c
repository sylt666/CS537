#include <stdio.h>
#include <stdlib.h>

typedef struct _lock_t {
    // unsigned int locked;
    int ticket;
    int turn;
} lock_t;

void test(int* x)
{
    printf("%d", *x);
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

int main()
{
    lock_t* lock = malloc(sizeof(lock_t));
    lock->ticket = 0;

    for (int i=0; i<100; i++){
        int ret = fetch_and_add(&lock->ticket, 1);
        printf ("%d ", ret);
    }

    return 0;
}