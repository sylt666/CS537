implemented a lottery scheduler into xv6

made two system calls

- settickets(int number) : sets the number of tickets of the calling process
- getinfo(struct pstat *) : returns some information about all running processes, including how many times each has been chosen to run and the process ID of each
