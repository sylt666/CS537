#include "types.h"
#include "stat.h"
#include "user.h"

#define N  100

static void
putc(int fd, char c)
{
    write(fd, &c, 1);
}

static void
printint(int fd, int xx, int base, int sgn)
{
    static char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if(sgn && xx < 0){
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;
    do{
        buf[i++] = digits[x % base];
    }while((x /= base) != 0);
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
        putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, char *fmt, ...)
{
    char *s;
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
        c = fmt[i] & 0xff;
        if(state == 0){
            if(c == '%'){
                state = '%';
            } else {
                putc(fd, c);
            }
        } else if(state == '%'){
            if(c == 'd'){
                printint(fd, *ap, 10, 1);
                ap++;
            } else if(c == 'x' || c == 'p'){
                printint(fd, *ap, 16, 0);
                ap++;
            } else if(c == 's'){
                s = (char*)*ap;
                ap++;
                if(s == 0)
                    s = "(null)";
                while(*s != 0){
                    putc(fd, *s);
                    s++;
                }
            } else if(c == 'c'){
                putc(fd, *ap);
                ap++;
            } else if(c == '%'){
                putc(fd, c);
            } else {
                // Unknown % sequence.  Print it to draw attention.
                putc(fd, '%');
                putc(fd, c);
            }
            state = 0;
        }
    }
}


void
forktest(void) {
    int n, pid;

    printf(1, "fork test\n");

    for (n = 0; n < N; n++) {
        pid = fork();
        printf(1, "the %d pid is : %d\n", n, pid);
        if (pid < 0)
            break;
        if (pid == 0) {
            exit();
        }
    }

    if (n == N) {
        printf(1, "fork claimed to work N times!\n", N);
        exit();
    }

    for (; n > 0; n--) {
        if (wait() < 0) {
            printf(1, "wait stopped early\n");
            exit();
        }
    }

    if (wait() != -1) {
        printf(1, "wait got too many\n");
        exit();
    }

    printf(1, "fork test OK\n");
}

int
main(int argc, char *argv[]) {
    forktest();
    exit();
}
