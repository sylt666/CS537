#include "types.h"
#include "stat.h"
#include "user.h"
#include "date.h"

int
main(int argc, char *argv[])
{
  struct rtcdate r;
  if(date(&r)){
      printf(2,"date failed\n");
  }

  printf(2,"UTC time:  %d:%d:%d,%d/%d,%d\n",r.hour,r.minute,r.second,r.month,r.day,r.year);
  if(r.hour+8>23){
      r.hour-=16;
      r.day++;
  }
  else
    r.hour+=8;
  printf(2,"Beijing time:  %d:%d:%d,%d/%d,%d\n",r.hour,r.minute,r.second,r.month,r.day,r.year);

  exit();
}
