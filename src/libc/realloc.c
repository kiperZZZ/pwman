/*
 * realloc.c - Redimensionnement de blocs mémoire
 * 
 * Fonctionnement:
 * 1. Cas spéciaux: ptr=NULL → malloc(), size=0 → free()
 * 2. Si nouvelle taille ≤ taille actuelle: garde le bloc
 * 3. Sinon: alloue nouveau bloc, copie les données, libère l'ancien
 */

#include "libc/libc.h"

static block_header_t *get_header(void *ptr) {
    return (block_header_t *)((char *)ptr - sizeof(block_header_t));
}

static void copy_data(void *dest, const void *src, size_t size) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

void *realloc(void *ptr, size_t size) {
    if (ptr == (void *)0) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return (void *)0;
    }
    
    size_t aligned_size = ALIGN(size);
    block_header_t *header = get_header(ptr);
    size_t current_size = GET_SIZE(header);
    
    if (IS_FREE(header)) {
        return (void *)0;
    }
    
    if (current_size >= aligned_size) {
        return ptr;
    }
    
    void *new_ptr = malloc(size);
    if (new_ptr == (void *)0) {
        return (void *)0;
    }
    
    size_t copy_size = (current_size < aligned_size) ? current_size : aligned_size;
    copy_data(new_ptr, ptr, copy_size);
    
    free(ptr);
    return new_ptr;
}