// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  void *addr;
  int key = 1;
  int num_pages = 1;
  printf(1, "before calling shmgetat in tester\n");
  addr = shmgetat(key, num_pages);
  printf(1, "addr %d\n", addr);

  int count = shm_refcount(key);
  printf(1, "count %d\n",count);
  while(1){
  	
  }

  exit();
}
