#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
  uint nosyscallsopen = getopenedcount();
  printf(1, "Number of system calls to open since current boot = %d", nosyscallsopen); 
}
