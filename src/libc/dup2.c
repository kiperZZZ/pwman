/*
 * dup2.c - Appel système dup2()
 * 
 * dup2() duplique un descripteur de fichier vers un autre numéro.
 * Utilise le syscall 33 sur Linux x86_64.
 * 
 * Paramètres:
 * - oldfd: descripteur source
 * - newfd: descripteur destination
 * 
 * Retour: newfd en cas de succès, -1 en cas d'erreur
 */

#include "libc/libc.h"

int dup2(int oldfd, int newfd) {
    int result;
    __asm__ volatile (
        "mov $33, %%rax\n"
        "syscall\n"
        : "=a" (result)
        : "D" (oldfd), "S" (newfd)
        :
    );
    return result;
}