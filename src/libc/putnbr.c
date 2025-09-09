#include "libc/libc.h" 

int putnbr(int num) {
    if (num == 0) {
        return putchar('0');
    }
    
    // Gérer les nombres négatifs
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        if (putchar('-') < 0) return -1;
        // Attention au cas INT_MIN !
        if (num == -2147483648) {
            return puts("2147483648") - 1; // -1 pour le '\n' en plus
        }
        num = -num;
    }
    
    // Conversion récursive
    if (num >= 10) {
        if (putnbr(num / 10) < 0) return -1;
    }
    
    return putchar((num % 10) + '0');
}