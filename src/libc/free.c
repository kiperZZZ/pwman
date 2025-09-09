/*
 * free.c - Libération de mémoire
 * 
 * Fonctionnement:
 * 1. Récupère le header depuis le pointeur utilisateur
 * 2. Vérifie que le bloc n'est pas déjà libre (double-free)
 * 3. Marque le bloc comme libre (bit 0 = 0)
 * 4. Ajoute le bloc en tête de la free list
 */

#include "libc/libc.h"

typedef struct free_block {
    struct free_block *next;
} free_block_t;

extern free_block_t *free_list;

static block_header_t *get_header(void *ptr) {
    return (block_header_t *)((char *)ptr - sizeof(block_header_t));
}

static void *get_data(block_header_t *header) {
    return (char *)header + sizeof(block_header_t);
}

void free(void *ptr) {
    if (ptr == (void *)0) {
        return;
    }
    
    block_header_t *header = get_header(ptr);
    
    if (IS_FREE(header)) {
        return;
    }
    
    SET_FREE(header);
    
    free_block_t *new_free = (free_block_t *)get_data(header);
    new_free->next = free_list;
    free_list = new_free;
}