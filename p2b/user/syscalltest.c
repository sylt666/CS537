#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    //ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    //ls(argv[i]);
  exit();
}
