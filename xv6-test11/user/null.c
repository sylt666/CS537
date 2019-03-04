#include "types.h"
#include "user.h"

/* Tester for null dereference.
 * It tries to dereference a null pointer.
 * If it fails, it passes */
int main(int argc, char * argv[])
{
    int *p = 0;
    printf(1, "This null pointer dereference should fail.\n");
    printf(1, "P = ", *p);
    exit();
}