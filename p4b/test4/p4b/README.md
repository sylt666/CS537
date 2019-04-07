added real kernel threads to xv6

- defined a new system call to create a kernel thread called clone()
- created another system call to wait for a thread called join()
- used clone() to build a little thread library with a thread_create() call and lock_acquire() and lock_release() functions
