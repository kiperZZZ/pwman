/*
 * close.c - Appel système close()
 * 
 * close() ferme un descripteur de fichier.
 * Utilise le syscall 3 sur Linux x86_64.
 * 
 * Paramètres:
 * - fd: descripteur de fichier à fermer
 * 
 * Retour: 0 en cas de succès, -1 en cas d'erreur
 */

#include "libc/libc.h"

int close(int fd) {
    long ret;
    __asm__ volatile (
        "mov $3, %%rax\n"   // syscall: close
        "mov %1, %%rdi\n"   // fd
        "syscall\n"
        : "=a"(ret)
        : "r"((long)fd)
        : "rdi"
    );
    return ret;
}