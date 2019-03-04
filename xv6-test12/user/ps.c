#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int main(int argc, char *argv[])
{
    // check the number of arguments
	int stdout = 1;
	if (argc > 1) {
		printf(stdout, "Usage: ps\n");
		exit();
	}

    // make the syscall and populate the process info table
    struct ProcessInfo processInfoTable[NPROC];
    int numProcessesGotten = getprocs(processInfoTable);

    char* states[6];
    states[0] = "UNUSED";
    states[1] = "EMBRYO";
    states[2] = "SLEEPING";
    states[3] = "RUNNABLE";
    states[4] = "RUNNING";
    states[5] = "ZOMBIE";

    char *fmt = "%d  %s  %d  %s\n"; // format for printing processes
    printf(stdout, "PPID  STATE  SIZE  NAME\n");
    printf(stdout, "-----------------------\n");
    int i;
    for (i = 0; i < numProcessesGotten; i++) {

        // copy the name into a variable
        char name[16];
        strcpy(name, processInfoTable[i].name);
        // if it is the init process then the ppid is going to be -1
        int ppid = -1;
        if (strcmp(name, "init") != 0) {
            ppid = processInfoTable[i].ppid;
        }
        // find the specified state from the enum
        char* state = states[processInfoTable[i].state];
        int sz = processInfoTable[i].sz;
    	printf(stdout, fmt, ppid, state, sz, processInfoTable[i].name);

    }

	exit();
}
