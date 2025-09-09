/*
 * fork.c - Appel système fork()
 * 
 * fork() crée un nouveau processus en dupliquant le processus actuel.
 * Utilise le syscall 57 sur Linux x86_64.
 * 
 * Retour:
 * - Dans le processus parent: PID du processus enfant
 * - Dans le processus enfant: 0
 * - En cas d'erreur: -1
 */

#include "libc/libc.h"

int fork(void) {
    int result;
    __asm__ volatile (
        "mov $57, %%rax\n"
        "syscall\n"
        : "=a" (result)
        :
        :
    );
    return result;
}