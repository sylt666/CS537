#include "types.h"
#include "stat.h"
#include "user.h"
#include "ProcessInfo.h"

int
main(int argc, char **argv)
{
  enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
  static char *states[] = {
  [UNUSED]    "UNUSED",
  [EMBRYO]    "EMBRYO",
  [SLEEPING]  "SLEEPING",
  [RUNNABLE]  "RUNNABLE",
  [RUNNING]   "RUNNING",
  [ZOMBIE]    "ZOMBIE"
  };
	struct ProcessInfo procTable[64];
  int numProcs = getprocs(procTable);
  struct ProcessInfo *p = procTable;
  for(p = procTable; p < &procTable[numProcs]; p++) {
    printf(1, "%d %d %s %d %s", p->pid, p->ppid, states[p->state], p->sz, p->name);
    printf(1, "\n");
  }
  exit();
}

