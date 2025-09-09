/*
 * waitpid.c - Appel système waitpid()
 * 
 * waitpid() attend qu'un processus enfant change d'état.
 * Utilise le syscall 61 sur Linux x86_64.
 * 
 * Paramètres:
 * - pid: PID du processus à attendre (-1 pour n'importe quel enfant)
 * - status: pointeur pour récupérer le statut de sortie
 * - options: options d'attente (0 pour attente bloquante)
 * 
 * Retour: PID du processus qui a changé d'état, ou -1 en cas d'erreur
 */

#include "libc/libc.h"

int waitpid(int pid, int *status, int options) {
    int result;
    __asm__ volatile (
        "mov $61, %%rax\n"
        "syscall\n"
        : "=a" (result)
        : "D" (pid), "S" (status), "d" (options)
        :
    );
    return result;
}