/*
 * libc.h - Header pour l'allocateur mémoire simple
 * 
 * Structure de bloc mémoire:
 * [block_header_t][données utilisateur]
 * 
 * Le header contient la taille + bit d'état (bit 0):
 * - 0 = bloc libre
 * - 1 = bloc occupé
 * 
 * Free list: les blocs libres sont chaînés via leurs données
 */

#ifndef LIBC_H
#define LIBC_H

typedef unsigned long long size_t;
typedef long long ssize_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

// File flags for open()
#define O_RDONLY    0
#define O_WRONLY    1  
#define O_RDWR      2
#define O_CREAT     64
#define O_TRUNC     512

// Structure d'un header de bloc
typedef struct {
    size_t size; // Taille + bit d'état (bit 0: 0=libre, 1=occupé)
} block_header_t;

// Macros de base
#define ALIGN(size) (((size) + 15) & ~15)
#define GET_SIZE(h) ((h)->size & ~1)
#define IS_FREE(h) (((h)->size & 1) == 0)
#define SET_FREE(h) ((h)->size = GET_SIZE(h))
#define SET_USED(h) ((h)->size = GET_SIZE(h) | 1)

// Fonctions d'I/O
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int putchar(char c);
int puts(const char *str);
ssize_t getline(char **lineptr, size_t *n, int fd);
int close(int fd);
int open(const char *pathname, int flags, int mode);

// Fonctions de string
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
void *memcpy(void *dest, const void *src, size_t n);

// Fonctions utilitaires
int putnbr(int num);
int printf(const char *format, ...);
void exit(int status) __attribute__((noreturn));

// Fonctions de gestion de la mémoire
void *brk(void *addr);
void *sbrk(long increment);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *memset(void *s, int c, size_t n);

// Fonctions de processus
int execve(const char *pathname, char *const argv[], char *const envp[]);
int fork(void);
int waitpid(int pid, int *status, int options);
int getpid(void);
int pipe(int pipefd[2]);
int dup2(int oldfd, int newfd);
#endif
