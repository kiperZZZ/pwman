#include "libc/libc.h" 

#define GETLINE_INITIAL_SIZE 128

ssize_t getline(char **lineptr, size_t *n, int fd) {
    if (!lineptr || !n) return -1;
    
    // Allouer le buffer initial si nécessaire
    if (*lineptr == 0 || *n == 0) {
        // Pour cette mini libc, on va utiliser un buffer statique
        // Dans une vraie implémentation, il faudrait malloc
        static char static_buffer[GETLINE_INITIAL_SIZE];
        *lineptr = static_buffer;
        *n = GETLINE_INITIAL_SIZE;
    }
    
    size_t pos = 0;
    char c;
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        if (pos >= *n - 1) {
            // Buffer trop petit - dans une vraie implémentation, on realloc
            return -1;
        }
        
        (*lineptr)[pos++] = c;
        
        if (c == '\n') {
            break;
        }
    }
    
    if (bytes_read < 0) return -1;
    if (pos == 0 && bytes_read == 0) return -1; // EOF
    
    (*lineptr)[pos] = '\0';
    return pos;
}