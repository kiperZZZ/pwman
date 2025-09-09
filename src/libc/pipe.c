/*
 * pipe.c - Appel système pipe()
 * 
 * pipe() crée un pipe (tube) pour la communication inter-processus.
 * Utilise le syscall 22 sur Linux x86_64.
 * 
 * Paramètres:
 * - pipefd: tableau de 2 entiers pour recevoir les descripteurs
 *   pipefd[0] = lecture, pipefd[1] = écriture
 * 
 * Retour: 0 en cas de succès, -1 en cas d'erreur
 */

#include "libc/libc.h"

int pipe(int pipefd[2]) {
    int result;
    __asm__ volatile (
        "mov $22, %%rax\n"
        "syscall\n"
        : "=a" (result)
        : "D" (pipefd)
        :
    );
    return result;
}