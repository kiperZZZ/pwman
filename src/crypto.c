#include "pwman.h"

// --- Fonctions internes (static) ---

// Rotation de 32-bit vers la gauche
static uint32_t rotl32(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

// Lit 4 octets (little-endian) pour former un entier de 32-bit
static uint32_t pack4(const uint8_t *a) {
    return ((uint32_t)a[0]) | ((uint32_t)a[1] << 8) | ((uint32_t)a[2] << 16) | ((uint32_t)a[3] << 24);
}

// Le "quart de tour" ChaCha20, l'opération de base de l'algorithme
#define CHACHA20_QUARTERROUND(x, a, b, c, d) \
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 16); \
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 12); \
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 8); \
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 7);


// Génère le prochain bloc de 64 octets de keystream
static void chacha20_block_next(struct chacha20_context *ctx) {
    for (int i = 0; i < 16; i++) ctx->keystream32[i] = ctx->state[i];

    for (int i = 0; i < 10; i++) {
        CHACHA20_QUARTERROUND(ctx->keystream32, 0, 4, 8, 12)
        CHACHA20_QUARTERROUND(ctx->keystream32, 1, 5, 9, 13)
        CHACHA20_QUARTERROUND(ctx->keystream32, 2, 6, 10, 14)
        CHACHA20_QUARTERROUND(ctx->keystream32, 3, 7, 11, 15)
        CHACHA20_QUARTERROUND(ctx->keystream32, 0, 5, 10, 15)
        CHACHA20_QUARTERROUND(ctx->keystream32, 1, 6, 11, 12)
        CHACHA20_QUARTERROUND(ctx->keystream32, 2, 7, 8, 13)
        CHACHA20_QUARTERROUND(ctx->keystream32, 3, 4, 9, 14)
    }

    for (int i = 0; i < 16; i++) ctx->keystream32[i] += ctx->state[i];

    // Incrémente le compteur de bloc (les 32 bits de poids faible de l'état[12])
    ctx->state[12]++;
    if (0 == ctx->state[12]) {
        // Gère le report si le compteur dépasse 2^32
        ctx->state[13]++;
    }
}


/**
 * Prépare une clé de 32 octets (MASTER_KEY_LEN) à partir d'un mot de passe.
 * Tronque ou complète avec des zéros pour atteindre la taille requise.
 */
void normalize_key(const char *password, uint8_t *key_buffer) {
    size_t pass_len = strlen(password);
    size_t len_to_copy = (pass_len < MASTER_KEY_LEN) ? pass_len : MASTER_KEY_LEN;

    memcpy(key_buffer, password, len_to_copy);
    memset(key_buffer + len_to_copy, 0, MASTER_KEY_LEN - len_to_copy);
}

/**
 * Initialise l'état interne de ChaCha20 avec la clé, le nonce et un compteur.
 */
void chacha20_init_context(struct chacha20_context *ctx, const uint8_t key[], const uint8_t nonce[], uint64_t counter) {
    memset(ctx, 0, sizeof(struct chacha20_context));

    const uint8_t *magic = (uint8_t*)"expand 32-byte k";
    ctx->state[0] = pack4(magic + 0); ctx->state[1] = pack4(magic + 4);
    ctx->state[2] = pack4(magic + 8); ctx->state[3] = pack4(magic + 12);

    ctx->state[4] = pack4(key + 0); ctx->state[5] = pack4(key + 4);
    ctx->state[6] = pack4(key + 8); ctx->state[7] = pack4(key + 12);
    ctx->state[8] = pack4(key + 16); ctx->state[9] = pack4(key + 20);
    ctx->state[10] = pack4(key + 24); ctx->state[11] = pack4(key + 28);

    ctx->state[12] = (uint32_t)counter;
    ctx->state[13] = pack4(nonce + 0);
    ctx->state[14] = pack4(nonce + 4);
    ctx->state[15] = pack4(nonce + 8);

    ctx->position = 64; // Force la génération d'un nouveau bloc au premier appel
}

/**
 * Applique l'opération XOR entre le keystream ChaCha20 et un buffer de données.
 * C'est la fonction qui chiffre et déchiffre.
 */
void chacha20_xor(struct chacha20_context *ctx, uint8_t *bytes, size_t n_bytes) {
    uint8_t *keystream8 = (uint8_t*)ctx->keystream32;
    for (size_t i = 0; i < n_bytes; i++) {
        if (ctx->position >= 64) {
            chacha20_block_next(ctx);
            ctx->position = 0;
        }
        bytes[i] ^= keystream8[ctx->position];
        ctx->position++;
    }
}