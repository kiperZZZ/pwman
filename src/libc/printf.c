#include "libc/libc.h"

// Fonction helper pour écrire une chaîne
static int write_string(const char *str) {
    int count = 0;
    if (!str) str = "(null)";
    
    while (*str) {
        if (putchar(*str) < 0) return -1;
        str++;
        count++;
    }
    return count;
}

// Fonction helper pour compter les chiffres d'un nombre
static int count_digits(int num) {
    if (num == 0) return 1;
    
    int count = 0;
    if (num < 0) {
        count++; // Pour le signe '-'
        num = -num;
    }
    
    while (num > 0) {
        count++;
        num /= 10;
    }
    return count;
}

// Fonction helper pour afficher un nombre hexadécimal
static int print_hex(unsigned long num, int uppercase) {
    if (num == 0) {
        return (putchar('0') < 0) ? -1 : 1;
    }
    
    char buffer[20]; // Assez pour 64-bit hex
    int i = 0;
    char base = uppercase ? 'A' : 'a';
    
    // Convertir en hex (en ordre inverse)
    while (num > 0) {
        int digit = num % 16;
        if (digit < 10) {
            buffer[i++] = '0' + digit;
        } else {
            buffer[i++] = base + (digit - 10);
        }
        num /= 16;
    }
    
    // Afficher en ordre correct
    int count = 0;
    while (i > 0) {
        if (putchar(buffer[--i]) < 0) return -1;
        count++;
    }
    
    return count;
}

// Fonction principale printf
int printf(const char *format, ...) {
    if (!format) return -1;
    
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    int total_count = 0;
    const char *ptr = format;
    
    while (*ptr) {
        if (*ptr == '%' && *(ptr + 1)) {
            ptr++; // Skip '%'
            
            switch (*ptr) {
                case 's': {
                    char *str = __builtin_va_arg(args, char*);
                    int count = write_string(str);
                    if (count < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += count;
                    break;
                }
                
                case 'd': {
                    int num = __builtin_va_arg(args, int);
                    if (putnbr(num) < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += count_digits(num);
                    break;
                }
                
                case 'c': {
                    int c = __builtin_va_arg(args, int);
                    if (putchar((char)c) < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count++;
                    break;
                }
                
                case 'p': {
                    void *ptr_val = __builtin_va_arg(args, void*);
                    
                    // Afficher "0x"
                    if (putchar('0') < 0 || putchar('x') < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += 2;
                    
                    // Afficher l'adresse en hexadécimal
                    int count = print_hex((unsigned long)ptr_val, 0);
                    if (count < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += count;
                    break;
                }
                
                case 'x': {
                    unsigned int num = __builtin_va_arg(args, unsigned int);
                    int count = print_hex(num, 0);
                    if (count < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += count;
                    break;
                }
                
                case 'X': {
                    unsigned int num = __builtin_va_arg(args, unsigned int);
                    int count = print_hex(num, 1);
                    if (count < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += count;
                    break;
                }
                
                case '%': {
                    if (putchar('%') < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count++;
                    break;
                }
                
                default:
                    // Spécificateur non supporté
                    if (putchar('%') < 0 || putchar(*ptr) < 0) {
                        __builtin_va_end(args);
                        return -1;
                    }
                    total_count += 2;
                    break;
            }
        } else {
            // Caractère normal
            if (putchar(*ptr) < 0) {
                __builtin_va_end(args);
                return -1;
            }
            total_count++;
        }
        ptr++;
    }
    
    __builtin_va_end(args);
    return total_count;
}