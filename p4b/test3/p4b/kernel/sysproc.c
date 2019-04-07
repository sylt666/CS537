#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "spinlock.h"
struct spinlock glock;
int
sys_fork(void)
{
  return fork();
}

int
sys_clone(void)
{
	//TODO
	void * fcn = NULL;
	void * arg1 = NULL;
	void * arg2 = NULL;
	void * stack = NULL;
	int rc = 0;
	rc = argptr(0, (char **)&fcn, (int)sizeof(void *));
if(rc == -1) return -1;
	rc = argptr(1, (char **)&arg1, (int)sizeof(void *));
if(rc == -1) return -1;
	rc = argptr(2, (char **)&arg2, (int)sizeof(void *));
if(rc == -1) return -1;
	rc = argptr(3, (char **)&stack, (int)sizeof(void *));
	if(rc == -1) return -1;
	return clone(fcn, arg1, arg2, stack);
}

int
sys_join(void)
{
	//TODO
	void ** stack = NULL;
	if(argptr(0, (char **)&stack, sizeof(void *)) < 0) {
		//cprintf("----------\nTEST: Join returned -1\n---------------\n");
		return -1;
	}
	//cprintf("--------------\nTEST: stack in sys_join: %x\n----------------\n", stack);
	return join(stack);
}
int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;
  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  //acquire(&glock);
  if(growproc(n) < 0)
    //release(&tickslock);
    return -1;
  //release(&tickslock);
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
