#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define PGSIZE 4096

void
lock_init(lock_t *lk)
{
    lk->flag =0;
}

void
lock_acquire(lock_t *lk)
{
    while(xchg(&lk->flag,1) != 0)
        ;
}

void
lock_release(lock_t *lk)
{
    xchg(&lk->flag,0);
}

int
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
    lock_t lock;
    lock_init(&lock);
    lock_acquire(&lock);
    void * stack = malloc(PGSIZE*2);
    lock_release(&lock);

    if((uint)stack % PGSIZE){
        stack = stack + (PGSIZE - (uint)stack % PGSIZE);
    }

    int re = clone(start_routine, arg1, arg2, stack);

    return re;
}

int thread_join(){
	void *stack;
	int re= join(&stack);

	lock_t lock;
	lock_init(&lock);
	lock_acquire(&lock);
	free(stack);
	lock_release(&lock);

	return re;
}
