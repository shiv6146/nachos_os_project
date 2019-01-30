#include "syscall.h"

int main() {
    ForkExec("../build/makethreads");
    ForkExec("../build/getint");
    ForkExec("../build/getstring");
    return 0;
}