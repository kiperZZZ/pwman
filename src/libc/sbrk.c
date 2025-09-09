/*
 * sbrk.c - Extension incrémentale du heap
 * 
 * sbrk(increment) étend ou réduit le heap de 'increment' bytes.
 * sbrk(0) retourne l'adresse actuelle sans modification.
 * Utilise brk() en interne pour modifier le program break.
 */

#include "libc/libc.h"

void *sbrk(long increment) {
    static void *current_brk = 0;
    
    if (current_brk == 0) {
        current_brk = brk(0);
    }
    
    if (increment == 0) {
        return current_brk;
    }

    void *old_brk = current_brk;
    void *new_brk = (char *)current_brk + increment;
    current_brk = brk(new_brk);
    
    if (current_brk < new_brk) {
        return (void *)-1;
    }
    
    return old_brk;
}
