#include "libc/libc.h"

ssize_t readline(char *buf, size_t size) {
    ssize_t i = 0;
    while (i < size - 1) {
        ssize_t ret = read(0, &buf[i], 1);
        if (ret <= 0) return -1;
        if (buf[i] == '\n') {
            buf[i] = '\0';
            return i;
        }
        i++;
    }
    buf[i] = '\0';
    return i;
}