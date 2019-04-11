#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "param.h"
#define PGSIZE (4096)
// #define ORG_STACKS_SIZE 500

// int count_thread_create = 0;
void* org_stacks[NPROC];

// void* addthread[NPROC];

int
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
//   printf(1, "thread_create\n");
  // this routine should call malloc() to create a new user stack
  void* org_stack = malloc(PGSIZE); // printf(1, "malloc stack: %d\n", startstack);
  void* stack = org_stack; // printf(1, "stack: %d\n", stack);
  uint offset = (uint)stack % PGSIZE; // printf(1, "offset: %d\n", offset);
  if (offset != 0)
    stack = (void*) ((uint)stack + (4096 - offset));
  // printf(1, "offset stack: %d\n", stack);

  int ret_clone = clone(start_routine, arg1, arg2, stack);
  if (ret_clone != -1) {
    org_stacks[ret_clone % NPROC] = org_stack;
  }

  // use clone() to create the child thread and get it running
  // returns the newly created PID to the parent, -1 otherwise
  return ret_clone;
}

// int thread_join() call should also be created, which calls the underlying join() system call,
// frees the user stack, and then returns.
// It returns the waited-for PID (when successful), -1 otherwise.
int
thread_join(void)
{
  void *stack;
  int threadid = join(&stack);
  if(stack == NULL) return threadid;
  else {
    free(org_stacks[threadid % NPROC]);
  }

  return threadid;
}

