#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}



void spin()
{
        int i = 0;
  int j = 0;
  int k = 0;
        for(i = 0; i < 500; ++i)
        {
                for(j = 0; j < 200000; ++j)
                {
                        k = j % 10;
      k = k + 1;
    }
        }
}



void print(struct pstat *st)
{
   int i;
   for(i = 0; i < NPROC; i++) {
      if (st->inuse[i]) {
          printf(1, "pid: %d tickets: %d ticks: %d\n", st->pid[i], st->tickets[i], st->ticks[i]);
      }
   }
}



int
main(int argc, char *argv[])
{
  
   
   //check(getpinfo(&st) == 0, "getpinfo");
   //check(getpinfo(NULL) == -1, "getpinfo with bad pointer");
   //check(getpinfo((struct pstat *)1000000) == -1, "getpinfo with bad pointer"); 
   

	for(int i = 0; i<3; i++){
		int pid = fork();
		if(pid==0){
			printf(2, "child%d : settickets\n", i);
			settickets(10*(i+1));
			spin();
			spin();
			exit();
		}

	}
			sleep(50);
	 		struct pstat st;
			getpinfo(&st);
			//printf(1, "loop : %d\n", i);
			print(&st);

			sleep(300);
			getpinfo(&st);
			//printf(1, "loop : %d\n", i);
			print(&st);
   exit();
}
