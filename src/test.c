// This is NOT a Python script
// This is a C program!

#include <tcclib.h> /* include the "Simple libc header for TCC" */

extern int add(int a, int b);

extern void eadk_timing_msleep_int(int ms);

extern const char hello[];

int fib(int n) {
    if (n <= 2) {
        return 1;
    } else {
        return fib(n-1) + fib(n-2);
    }
}

int main(int n) {
    printf("%s\n", hello);
    eadk_timing_msleep_int(1000);
    printf("fib(%d) = %d\n", n, fib(n));
    eadk_timing_msleep_int(1000);
    printf("add(%d, %d) = %d\n", n, 2 * n, add(n, 2 * n));
    eadk_timing_msleep_int(1000);
    return 0;
}