/*
 * execve.c - Appel système execve()
 * 
 * execve() remplace l'image du processus actuel par un nouveau programme.
 * Utilise le syscall 59 sur Linux x86_64.
 * Si l'appel réussit, il ne retourne pas (le processus est remplacé).
 * Si l'appel échoue, il retourne -1.
 */

#include "libc/libc.h"

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    int result;
    __asm__ volatile (
        "mov $59, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "syscall\n"
        : "=a" (result)
        : "D" (pathname), "S" (argv), "d" (envp)
        :
    );
    return result;
}