/*
 * getpid.c - Appel système getpid()
 * 
 * getpid() retourne l'ID du processus actuel.
 * Utilise le syscall 39 sur Linux x86_64.
 */

#include "libc/libc.h"

int getpid(void) {
    int result;
    __asm__ volatile (
        "mov $39, %%rax\n"
        "syscall\n"
        : "=a" (result)
        :
        :
    );
    return result;
}