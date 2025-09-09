#include "libc/libc.h"

ssize_t read(int fd, void *buf, size_t count) {
    ssize_t ret;

    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a"(0L),
          "D"((long)fd),
          "S"(buf),
          "d"(count)
        : "rcx", "r11", "memory"
    );
    return ret;
}