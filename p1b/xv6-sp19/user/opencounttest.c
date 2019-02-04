#include "types.h"
#include "user.h"

int stdout = 1;

int
main(int argc, char *argv[])
{
  int fd;
  uint numberOpenCalls = getopenedcount();

  printf(1, "open count before = %d\n", numberOpenCalls);

  fd = open("echo", 0);
  if(fd < 0){
      printf(stdout, "open echo failed!\n");
      exit();
  }

  numberOpenCalls = getopenedcount();
  printf(1, "open count after = %d\n", numberOpenCalls);
  
  exit();
}
