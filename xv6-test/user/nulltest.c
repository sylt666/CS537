#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int* i = malloc(sizeof(int)*1);
    int* b = NULL;
    free(i);
    printf(1, "%s", *b);
    printf(1, "success");
    exit();
}