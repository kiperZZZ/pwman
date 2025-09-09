#include "libc/libc.h"

size_t strlen(const char *s) {
    size_t i = 0;
    while (s && s[i]) {
        i++;
    }
    return i;
}