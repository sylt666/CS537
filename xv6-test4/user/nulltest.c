#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "Read byte at address 0: %p\n", *((int*)0x1000));
    printf(1, "Read byte at address 0: %p\n", *((int*)0x0FFF));
    exit();
}
