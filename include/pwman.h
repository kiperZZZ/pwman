#ifndef PWMAN_H
#define PWMAN_H

#include "libc/libc.h"

typedef unsigned char  uint8_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;

#define MAX_ENTRIES 100
#define MAX_NAME_LEN 64
#define MAX_PLATFORM_LEN 64
#define MAX_USER_LEN 64
#define MAX_PASSWORD_LEN 64
#define MASTER_KEY_LEN 32      
#define CHACHA20_NONCE_LEN 12  

typedef struct {
    char name[MAX_NAME_LEN];
    char platform[MAX_PLATFORM_LEN];
    char user[MAX_USER_LEN];
    char password[MAX_PASSWORD_LEN];
} PwEntry;

typedef struct {
    int count;
    PwEntry entries[MAX_ENTRIES];
} Vault;

typedef struct {
    uint8_t nonce[CHACHA20_NONCE_LEN];
    uint8_t encrypted_vault[sizeof(Vault)];
} VaultFileLayout;

struct chacha20_context
{
    uint32_t keystream32[16];
    size_t position;
    uint32_t state[16];
};

void normalize_key(const char *password, uint8_t *key_buffer);
void chacha20_init_context(struct chacha20_context *ctx, const uint8_t key[], const uint8_t nonce[], uint64_t counter);
void chacha20_xor(struct chacha20_context *ctx, uint8_t *bytes, size_t n_bytes);

int save_vault(const char *filepath, Vault *vault, const char *master_password);
int load_vault(const char *filepath, Vault *vault, const char *master_password);

#endif 