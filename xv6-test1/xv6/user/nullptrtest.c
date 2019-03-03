#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    printf(1, "Read from 0x1000: %p\n", *((int*)0x1000));
    printf(1, "Read from 0x0FFF: %p\n", *((int*)0x0FFF));
    exit();
}
