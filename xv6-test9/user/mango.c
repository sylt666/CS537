#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];

void
cat(int fd)
{
  int n;

  while((n = read(fd, buf, sizeof(buf))) > 0)
    write(1, buf, n);
  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd;
  if((fd = open("logo", 0)) < 0){
    printf(1, "cat: cannot open logo\n");
    exit();
  }
  cat(fd);
  close(fd);
  exit();
}