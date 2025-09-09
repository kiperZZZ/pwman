#include "libc/libc.h"

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    
    void *original_dest = dest;

    if (n < 8) {
        while (n--) {
            *d++ = *s++;
        }
        return original_dest;
    }

    // 1. Copier les premiers octets un par un jusqu'à ce que la destination soit alignée
    while (((unsigned long)d % sizeof(unsigned long)) != 0) {
        *d++ = *s++;
        n--;
    }

    // 2. Copier la majeure partie des données par blocs de 8 octets (64 bits).
    unsigned long *wd = (unsigned long *)d;
    const unsigned long *ws = (const unsigned long *)s;
    while (n >= sizeof(unsigned long)) {
        *wd++ = *ws++;
        n -= sizeof(unsigned long);
    }

    d = (char *)wd;
    s = (const char *)ws;
    while (n--) {
        *d++ = *s++;
    }

    return original_dest;
}