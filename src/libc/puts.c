#include "libc/libc.h" 

int puts(const char *str) {
    if (!str) return -1;
    
    // Calculer la longueur de la chaîne
    size_t len = 0;
    while (str[len]) len++;
    
    // Écrire la chaîne
    if (write(1, str, len) < 0) return -1;
    
    // Ajouter le '\n'
    if (putchar('\n') < 0) return -1;
    
    return len + 1;
}