#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  struct proc * que[4][NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  p->priority = 3;
  p->ticks[3] = 0;
  p->ticks[2] = 0;
  p->ticks[1] = 0;
  p->ticks[0] = 0;
  p->wait_ticks[3] = 0;
  p->wait_ticks[2] = 0;
  p->wait_ticks[1] = 0;
  p->wait_ticks[0] = 0;
  //Add Process to the Queue 0 of MLFQ
  struct proc **pr;
  for(pr = &(ptable.que[3][0]); pr < &ptable.que[3][NPROC] ; pr++){
    if(*pr == NULL){
      *pr = p;
      break;
    }
  }
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  np->priority = 3;
  np->ticks[3] = 0;
  np->ticks[2] = 0;
  np->ticks[1] = 0;
  np->ticks[0] = 0;
  np->wait_ticks[3] = 0;
  np->wait_ticks[2] = 0;
  np->wait_ticks[1] = 0;
  np->wait_ticks[0] = 0;
  //Adding process to the queue 0
  acquire(&ptable.lock);
  struct proc **pr;
  for(pr = &(ptable.que[3][0]); pr < &ptable.que[3][NPROC] ; pr++){
    if(*pr == NULL){
      *pr = np;
      break;
    }
  }

  release(&ptable.lock);
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().

  
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

void starvHandler(void){
  //struct proc *p1;
  //struct proc **pr1;
  struct proc *p;
  struct proc **pr;
  struct proc *q;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state != RUNNABLE)
      continue;
    p->wait_ticks[p->priority]++;
    if(p->priority == 0 && p->wait_ticks[0] >= 500){
      //Process needs to be promoted to Priority 1
      for(pr = &(ptable.que[0][0]); pr < &(ptable.que[0][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        q = *pr;
        if(q->pid == p->pid){
          *pr = NULL;
          q->priority++;
          struct proc **next;
          for(next = &(ptable.que[1][0]); next < &(ptable.que[1][NPROC]) ; next++){
            if(*next == NULL){
              q->ticks[1]=0;
              q->wait_ticks[1]=0;
              *next = q;
              break;
            }
          }
        }
      }
    }

    if(p->priority == 1 && p->wait_ticks[1] >= 320){
      //Process needs to be promoted to Priority 2
      for(pr = &(ptable.que[1][0]); pr < &(ptable.que[1][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        q = *pr;
        if(q->pid == p->pid){
          *pr = NULL;
          q->priority++;
          struct proc **next;
          for(next = &(ptable.que[2][0]); next < &(ptable.que[2][NPROC]) ; next++){
            if(*next == NULL){
              q->ticks[2]=0;
              q->wait_ticks[2]=0;
              *next = q;
              break;
            }
          }
        }
      }
    }

    if(p->priority == 2 && p->wait_ticks[1] >= 160){
      //Process needs to be promoted to Priority 2
      for(pr = &(ptable.que[2][0]); pr < &(ptable.que[2][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        q = *pr;
        if(q->pid == p->pid){
          *pr = NULL;
          q->priority++;
          struct proc **next;
          for(next = &(ptable.que[3][0]); next < &(ptable.que[3][NPROC]) ; next++){
            if(*next == NULL){
              q->ticks[3]=0;
              q->wait_ticks[3]=0;
              *next = q;
              break;
            }
          }
        }
      }
    }
  }
}
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct proc **pr;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(pr = &(ptable.que[3][0]); pr < &(ptable.que[3][NPROC]) ; pr++){
      if((*pr) == NULL)
        continue;
      if((*pr)->state == UNUSED){
        *pr = NULL;
        continue;
      }
      if((*pr)->state != RUNNABLE)
        continue;
      p = *pr;
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      p->ticks[3]++;
      p->wait_ticks[3]=0;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();
      
      if(p->ticks[3] >= 8){
        *pr = NULL;
        p->priority--;
        struct proc **next;
        for(next = &(ptable.que[2][0]); next < &(ptable.que[2][NPROC]) ; next++){
          if(*next == NULL){
            p->wait_ticks[2]=0;
            *next = p;
            break;
          }
        }
      }  
      starvHandler();
      proc = 0;
      break;
    }
    
    if(pr == &(ptable.que[3][NPROC])){
      for(pr = &(ptable.que[2][0]); pr < &(ptable.que[2][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        if((*pr)->state == UNUSED){
          *pr = NULL;
          continue;
        }
        if((*pr)->state != RUNNABLE)
          continue;
        p = *pr;
        proc = p;
        switchuvm(p);
        p->state = RUNNING;
        
        p->wait_ticks[2]=0;
        swtch(&cpu->scheduler, proc->context);
        switchkvm();
        p->ticks[2]++;
        if(p->ticks[2] >= 16){
          *pr = NULL;
          p->priority--;
          struct proc **next;
          for(next = &(ptable.que[1][0]); next < &(ptable.que[1][NPROC]) ; next++){
            if(*next == NULL){
              p->wait_ticks[1]=0;
              *next = p;
              break;
            }
          }
        }  
        starvHandler();
        proc = 0;
        break;
      }
    }

    if(pr == &(ptable.que[2][NPROC])){
      for(pr = &(ptable.que[1][0]); pr < &(ptable.que[1][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        if((*pr)->state == UNUSED){
          *pr = NULL;
          continue;
        }
        if((*pr)->state != RUNNABLE)
          continue;
        p = *pr;
        proc = p;
        switchuvm(p);
        p->state = RUNNING;
        
        p->wait_ticks[1]=0;
        swtch(&cpu->scheduler, proc->context);
        switchkvm();
        p->ticks[1]++;
        if(p->ticks[1] >= 32){
          *pr = NULL;
          p->priority--;
          struct proc **next;
          for(next = &(ptable.que[0][0]); next < &(ptable.que[0][NPROC]) ; next++){
            if(*next == NULL){
            p->wait_ticks[0]=0;
            *next = p;
            break;
            }
          }
        }   
        starvHandler();
        proc = 0;
        break;
      }
    }

    if(pr == &(ptable.que[1][NPROC])){
      for(pr = &(ptable.que[0][0]); pr < &(ptable.que[0][NPROC]) ; pr++){
        if((*pr) == NULL)
          continue;
        if((*pr)->state == UNUSED){
          *pr = NULL;
          continue;
        }
        if((*pr)->state != RUNNABLE)
          continue;
        p = *pr;
        proc = p;
        switchuvm(p);
        p->state = RUNNING;
        
        p->wait_ticks[0]=0;
        swtch(&cpu->scheduler, proc->context);
        switchkvm();
        p->ticks[0]++;
        if(p->state == UNUSED || p->state == ZOMBIE || p->state == SLEEPING){
          starvHandler();
          break;
        } 
        else{
          pr--;
        }
        starvHandler();
        proc = 0;
      }
    }
    
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int 
getpinfo(struct pstat *a){
  struct proc *p;
  int idx = 0;
  acquire(&ptable.lock);
  // Loop over process table looking for process to run.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED) {
      a->inuse[idx] = 0;
      idx++;
    } else {
      a->inuse[idx] = 1;
      a->pid[idx] = p->pid;
      a->state[idx] = p->state;
      a->priority[idx] = p->priority;
      int i = 0;
      for (i = 0; i < 4; i++) {
        a->ticks[idx][i] = p->ticks[i];
        a->wait_ticks[idx][i] = p->wait_ticks[i];
      }
      idx++;
    }
  }
  release(&ptable.lock);
  return 0;
} 