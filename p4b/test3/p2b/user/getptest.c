#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}



int
main(int argc, char *argv[])
{
  
   
   //check(getpinfo(&st) == 0, "getpinfo");
   check(getpinfo(NULL) == -1, "getpinfo with bad pointer");
   //check(getpinfo((struct pstat *)1000000) == -1, "getpinfo with bad pointer"); 
   exit();
}
