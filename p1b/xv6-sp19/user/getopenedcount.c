#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  uint numberOpenCalls = getopenedcount();
  printf(1, "Number of open calls this boot is = %d\n", numberOpenCalls);
  exit();
}
