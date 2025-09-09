/*
 * malloc.c - Allocation de mémoire avec free list
 * 
 * Fonctionnement:
 * 1. Cherche un bloc libre dans la free list (first-fit)
 * 2. Si trouvé: le retire de la liste et le marque occupé
 * 3. Sinon: étend le heap avec sbrk() et crée un nouveau bloc
 * 
 * Tous les blocs sont alignés sur 16 bytes
 */

#include "libc/libc.h"

typedef struct free_block {
    struct free_block *next;
} free_block_t;

free_block_t *free_list = (void *)0;

static block_header_t *get_header(void *ptr) {
    return (block_header_t *)((char *)ptr - sizeof(block_header_t));
}

static void *get_data(block_header_t *header) {
    return (char *)header + sizeof(block_header_t);
}

static block_header_t *find_free_block(size_t size) {
    free_block_t *current = free_list;
    free_block_t *prev = (void *)0;
    
    while (current != (void *)0) {
        block_header_t *header = get_header(current);
        if (GET_SIZE(header) >= size) {
            if (prev == (void *)0) {
                free_list = current->next;
            } else {
                prev->next = current->next;
            }
            return header;
        }
        prev = current;
        current = current->next;
    }
    return (void *)0;
}

void *malloc(size_t size) {
    if (size == 0) size = 8;
    
    size_t aligned_size = ALIGN(size);
    
    block_header_t *header = find_free_block(aligned_size);
    
    if (header != (void *)0) {
        SET_USED(header);
        return get_data(header);
    }
    
    size_t total_size = ALIGN(sizeof(block_header_t) + aligned_size);
    void *new_memory = sbrk(total_size);
    if (new_memory == (void *)-1) {
        return (void *)0;
    }
    
    header = (block_header_t *)new_memory;
    header->size = aligned_size | 1;
    
    return get_data(header);
}
