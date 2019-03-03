#include "types.h"
#include "stat.h"
#include "user.h"

void
test_failed()
{
	printf(1, "TEST FAILED\n");
	exit();
}

void
test_passed()
{
 printf(1, "TEST PASSED\n");
 exit();
}

int
main(int argc, char *argv[])
{
	void *ptr;

	ptr = (char *) shmgetat(3, 1);
	if (ptr == (char *) -1) {
		test_failed();
	}
        printf(1, "1 = %d\n", ptr);
	int pid = fork();
	if (pid < 0) {
		test_failed();
	}
	else if (pid == 0) {
		void *ptr2, *ptr3;

		ptr2 = (char *) shmgetat(3, 3);
		if (ptr2 == (char *) -1) {
			test_failed();
		}
                printf(1, "2 = %d\n", ptr2);
		if (ptr != ptr2) {
			test_failed();
		}
                printf(1, "a\n");
                ptr3 = (char *) shmgetat(4, 3);
		  if (ptr3 == (char *) -1) {
			test_failed();
		  }
    printf(1, "3 = %d\n", ptr3);
    if (ptr3 >= ptr2) {
      test_failed();
    }

		exit();
	}
	else {
		wait();
	}

	test_passed();
	exit();
}
