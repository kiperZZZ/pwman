/*
 * brk.c - Appel système brk()
 * 
 * brk() définit la fin du segment de données (program break).
 * Utilise le syscall 12 sur Linux x86_64.
 * Retourne la nouvelle adresse du program break.
 */

#include "libc/libc.h"

void *brk(void *addr) {
    void *result;
    __asm__ volatile (
        "mov $12, %%rax\n"
        "mov %1, %%rdi\n"
        "syscall\n"
        : "=a" (result)
        : "D" (addr)
        : 
    );
    return result;
}