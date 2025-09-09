#include "libc/libc.h" 

int putchar(char c) {
    ssize_t ret = write(1, &c, 1);
    return (ret == 1) ? (int)c : -1;
}