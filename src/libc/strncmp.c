#include "libc/libc.h" 

int strncmp(const char *s1, const char *s2, size_t n) {
    if (!s1 || !s2 || n == 0) {
        if (n == 0) return 0;
        return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    }
    
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}