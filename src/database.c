#include "pwman.h"

int save_vault(const char *filepath, Vault *vault, const char *master_password) {
    VaultFileLayout layout;
    uint8_t key[MASTER_KEY_LEN];
    struct chacha20_context ctx;

    int urandom_fd = open("/dev/urandom", O_RDONLY, 0);
    if (urandom_fd < 0) {
        puts("Erreur: Impossible d'ouvrir /dev/urandom.\n");
        return -1;
    }
    if (read(urandom_fd, layout.nonce, CHACHA20_NONCE_LEN) != CHACHA20_NONCE_LEN) {
        puts("Erreur: Impossible de lire le nonce depuis /dev/urandom.\n");
        close(urandom_fd);
        return -1;
    }
    close(urandom_fd);

    normalize_key(master_password, key);
    memcpy(layout.encrypted_vault, vault, sizeof(Vault));
    chacha20_init_context(&ctx, key, layout.nonce, 0);
    chacha20_xor(&ctx, layout.encrypted_vault, sizeof(Vault));

    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        puts("Erreur: Impossible de créer ou d'ouvrir le fichier de coffre-fort.\n");
        return -1;
    }
    
    ssize_t bytes_written = write(fd, &layout, sizeof(VaultFileLayout));
    close(fd);

    if (bytes_written != sizeof(VaultFileLayout)) {
        puts("Erreur lors de l'écriture dans le fichier de coffre-fort.\n");
        return -1;
    }

    return 0;
}

int load_vault(const char *filepath, Vault *vault, const char *master_password) {
    VaultFileLayout layout;
    uint8_t key[MASTER_KEY_LEN];
    struct chacha20_context ctx;

    int fd = open(filepath, O_RDONLY, 0);
    if (fd < 0) {
        return -1;
    }
    ssize_t bytes_read = read(fd, &layout, sizeof(VaultFileLayout));
    close(fd);

    if (bytes_read != sizeof(VaultFileLayout)) {
        puts("Erreur: Fichier de coffre-fort corrompu ou de taille incorrecte.\n");
        return -1;
    }

    normalize_key(master_password, key);
    chacha20_init_context(&ctx, key, layout.nonce, 0);
    chacha20_xor(&ctx, layout.encrypted_vault, sizeof(Vault));
    memcpy(vault, layout.encrypted_vault, sizeof(Vault));
    
    // Validate vault data to detect wrong password/corrupted data
    if (vault->count < 0 || vault->count > MAX_ENTRIES) {
        puts("Error: Invalid vault data. Wrong password or corrupted file.\n");
        return -1;
    }
    
    return 0;
}