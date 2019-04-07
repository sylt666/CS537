For this project, first, two system call were added: int getpinfo(struct pstat *) and int settickets(int num). The sys_getpinfo function was defined in sysproc.c, which would read the pstat parameter with argptr function. After get the pstat parameter, it passes it to a help function getpinfo that was defined in proc.c. The helper function then store all the information about current processes into the pstat. Function settickets is defined in sysproc.c. It read the parameter with argint and then set ticket with that value. Both two functions are declared appropriately in header files, including sysfunc.h, syscall.h, syscall.c, user.h and usys.S. The help functions are declared in defs.h.

Second, the default scheduler was modified to a lottery scheduler. The lottery scheduler follows the design introduced in the text book, and a random number generator lfsr113() was used, which I got from http://www.iro.umontreal.ca/~simardr/rng/lfsr113.c.


