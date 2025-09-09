#include "libc/libc.h"

int open(const char *pathname, int flags, int mode) {
    int ret;

    register int mode_reg asm("r10") = mode;

    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a"(257L),
          "D"(-100L),
          "S"(pathname),
          "d"(flags),
          "r"(mode_reg)
        : "rcx", "r11", "memory"
    );
    return ret;
}
