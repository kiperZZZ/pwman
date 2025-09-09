#include "libc/libc.h"

ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t ret;

    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a"(1L),
          "D"((long)fd),
          "S"(buf),
          "d"(count)
        : "rcx", "r11", "memory"
    );
    return ret;
}
