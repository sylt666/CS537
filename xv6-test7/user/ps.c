#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "ProcessInfo.h"

int
main(int argc, char *argv[])
{
  enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
  char *states[] = {
  	[UNUSED] 	  "UNUSED",
  	[EMBRYO] 	  "EMBRYO",
    [SLEEPING]  "SLEEPING",
    [RUNNABLE]  "RUNNABLE",
    [RUNNING] 	"RUNNING",
    [ZOMBIE]  	"ZOMBIE"
  };
  int i;
  struct ProcessInfo processInfoTable[NPROC];
  int numberOfCurrentProcesses = getprocs(processInfoTable);
  for (i = 0; i < numberOfCurrentProcesses; i++){
    printf(1, "%d %d %s %d %s\n", processInfoTable[i].pid, processInfoTable[i].ppid, states[processInfoTable[i].state], processInfoTable[i].sz, processInfoTable[i].name);
  }

  exit();
}
