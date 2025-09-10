# PWMan - Architecture and Code Analysis

## Table of Contents
1. [Project Overview](#project-overview)
2. [Architecture Overview](#architecture-overview)
3. [Data Structures](#data-structures)
4. [Cryptographic Implementation](#cryptographic-implementation)
5. [Database Management](#database-management)
6. [User Interface](#user-interface)
7. [Build System](#build-system)
8. [Security Analysis](#security-analysis)
9. [Flow Diagrams](#flow-diagrams)

---

## Project Overview

PWMan is a minimalist password manager implemented entirely in C for Linux x86_64. The project demonstrates low-level system programming by:

- **No standard library dependencies**: Compiled with `-nostdlib -fno-builtin`
- **Direct system calls**: All OS interactions through custom libc implementation
- **Custom memory management**: Manual heap management with `malloc`/`free`
- **ChaCha20 encryption**: Modern symmetric encryption for data protection
- **Simple file format**: Binary vault storage with encryption

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        PWMan Architecture                    │
├─────────────────────────────────────────────────────────────┤
│  CLI Interface (main.c)                                     │
│  ├─ Command parsing (init, list, get, add)                  │
│  ├─ User input handling                                     │
│  └─ Error handling and usage display                       │
├─────────────────────────────────────────────────────────────┤
│  Database Layer (database.c)                               │
│  ├─ Vault serialization/deserialization                    │
│  ├─ File I/O operations                                     │
│  └─ Encryption/Decryption coordination                     │
├─────────────────────────────────────────────────────────────┤
│  Cryptographic Layer (crypto.c)                            │
│  ├─ ChaCha20 stream cipher implementation                   │
│  ├─ Key derivation and normalization                       │
│  └─ Random nonce generation                                │
├─────────────────────────────────────────────────────────────┤
│  Custom LibC (src/libc/)                                   │
│  ├─ Memory management (malloc, free, sbrk)                 │
│  ├─ I/O operations (read, write, open, close)              │
│  ├─ String operations (strcmp, strlen, memcpy)             │
│  └─ System calls wrappers                                  │
├─────────────────────────────────────────────────────────────┤
│  Assembly Runtime (crt0.asm)                               │
│  └─ Program entry point and stack setup                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Data Structures

### Core Data Types

```c
// Basic types (no stdint.h available)
typedef unsigned char  uint8_t;    // 1 byte
typedef unsigned int   uint32_t;   // 4 bytes  
typedef unsigned long long uint64_t; // 8 bytes
```

### Password Entry Structure

```c
typedef struct {
    char name[MAX_NAME_LEN];         // 64 bytes - Entry identifier
    char platform[MAX_PLATFORM_LEN]; // 64 bytes - Service/website
    char user[MAX_USER_LEN];         // 64 bytes - Username
    char password[MAX_PASSWORD_LEN]; // 64 bytes - Password
} PwEntry;
// Total size: 256 bytes per entry
```

**Design Decisions:**
- **Fixed-size fields**: Simplifies memory management and serialization
- **Generous buffer sizes**: 64 bytes each to handle most real-world scenarios
- **No dynamic allocation**: Avoids complexity in vault serialization
- **Cache-friendly**: Power-of-2 size (256 bytes) aligns well with CPU cache lines

### Vault Container

```c
typedef struct {
    int count;                    // 4 bytes - Number of stored entries
    PwEntry entries[MAX_ENTRIES]; // 25,600 bytes - Array of entries (100 × 256)
} Vault;
// Total size: 25,604 bytes (~25KB)
```

**Memory Layout:**
```
Vault Structure (25,604 bytes):
┌─────────────┐ Offset 0
│ count (int) │ 4 bytes
├─────────────┤ Offset 4
│ entries[0]  │ 256 bytes
├─────────────┤ Offset 260
│ entries[1]  │ 256 bytes
├─────────────┤ Offset 516
│     ...     │
├─────────────┤
│entries[99]  │ 256 bytes
└─────────────┘ Offset 25,604
```

### File Storage Format

```c
typedef struct {
    uint8_t nonce[CHACHA20_NONCE_LEN];      // 12 bytes - Random nonce
    uint8_t encrypted_vault[sizeof(Vault)]; // 25,604 bytes - Encrypted vault data
} VaultFileLayout;
// Total file size: 25,616 bytes
```

**File Structure:**
```
Vault File Layout (25,616 bytes):
┌─────────────────┐ Offset 0
│ Nonce (12 bytes)│ Random value for ChaCha20
├─────────────────┤ Offset 12
│                 │
│  Encrypted      │ ChaCha20(Vault, Key, Nonce)
│  Vault Data     │ Contains count + all entries
│  (25,604 bytes) │
│                 │
└─────────────────┘ Offset 25,616
```

### ChaCha20 Context

```c
struct chacha20_context {
    uint32_t keystream32[16];  // 64 bytes - Current keystream block
    size_t position;           // 8 bytes - Position within current block
    uint32_t state[16];        // 64 bytes - ChaCha20 internal state
};
// Total size: 136 bytes
```

**ChaCha20 State Layout:**
```
ChaCha20 State (64 bytes / 16 words):
┌───────┬───────┬───────┬───────┐
│ Word0 │ Word1 │ Word2 │ Word3 │ Constants ("expand 32-byte k")
├───────┼───────┼───────┼───────┤
│ Word4 │ Word5 │ Word6 │ Word7 │ Key (first 16 bytes)
├───────┼───────┼───────┼───────┤
│ Word8 │ Word9 │Word10 │Word11 │ Key (last 16 bytes)  
├───────┼───────┼───────┼───────┤
│Word12 │Word13 │Word14 │Word15 │ Counter + Nonce
└───────┴───────┴───────┴───────┘
```

---

## Cryptographic Implementation

### ChaCha20 Stream Cipher

PWMan implements the ChaCha20 stream cipher, a modern and secure encryption algorithm designed by Daniel J. Bernstein.

#### Key Components

**1. Quarter Round Function**
```c
#define CHACHA20_QUARTERROUND(x, a, b, c, d) \
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 16); \
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 12); \
    x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 8);  \
    x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 7);
```

This macro performs the core ChaCha20 operation on 4 words of the state:
- **Addition**: Modular addition of 32-bit integers
- **XOR**: Bitwise exclusive OR
- **Rotation**: Left rotation by fixed amounts (16, 12, 8, 7)

**2. Block Generation**
```c
static void chacha20_block_next(struct chacha20_context *ctx) {
    // 1. Copy initial state to working keystream
    for (int i = 0; i < 16; i++) 
        ctx->keystream32[i] = ctx->state[i];

    // 2. Apply 20 rounds (10 double-rounds)
    for (int i = 0; i < 10; i++) {
        // Column rounds
        CHACHA20_QUARTERROUND(ctx->keystream32, 0, 4, 8, 12)
        CHACHA20_QUARTERROUND(ctx->keystream32, 1, 5, 9, 13)
        CHACHA20_QUARTERROUND(ctx->keystream32, 2, 6, 10, 14)
        CHACHA20_QUARTERROUND(ctx->keystream32, 3, 7, 11, 15)
        
        // Diagonal rounds
        CHACHA20_QUARTERROUND(ctx->keystream32, 0, 5, 10, 15)
        CHACHA20_QUARTERROUND(ctx->keystream32, 1, 6, 11, 12)
        CHACHA20_QUARTERROUND(ctx->keystream32, 2, 7, 8, 13)
        CHACHA20_QUARTERROUND(ctx->keystream32, 3, 4, 9, 14)
    }

    // 3. Add original state (prevents length extension attacks)
    for (int i = 0; i < 16; i++) 
        ctx->keystream32[i] += ctx->state[i];

    // 4. Increment block counter
    ctx->state[12]++;
    if (ctx->state[12] == 0) ctx->state[13]++; // Handle overflow
}
```

**3. Key Stream Generation**
```c
void chacha20_xor(struct chacha20_context *ctx, uint8_t *bytes, size_t n_bytes) {
    uint8_t *keystream8 = (uint8_t*)ctx->keystream32;
    
    for (size_t i = 0; i < n_bytes; i++) {
        // Generate new block if current one is exhausted
        if (ctx->position >= 64) {
            chacha20_block_next(ctx);
            ctx->position = 0;
        }
        
        // XOR data with keystream
        bytes[i] ^= keystream8[ctx->position];
        ctx->position++;
    }
}
```

#### Security Properties

**Strengths:**
- **Modern design**: Resistant to known cryptanalytic attacks
- **Performance**: Optimized for software implementation  
- **Proven security**: Widely adopted (TLS, SSH, etc.)
- **Side-channel resistance**: Constant-time operations

**Implementation notes:**
- **Nonce uniqueness**: Each vault file uses a fresh random nonce
- **Key derivation**: Master password normalized to 32 bytes
- **Stream cipher**: Same operation for encryption and decryption

---

## Database Management

### Vault Persistence Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   Vault Persistence Flow               │
└─────────────────────────────────────────────────────────┘

Save Operation:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Vault     │────│  Generate   │────│   Encrypt   │────│  Write to   │
│ Structure   │    │   Nonce     │    │    with     │    │    File     │
│   (RAM)     │    │(/dev/urandom│    │  ChaCha20   │    │   (Disk)    │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘

Load Operation:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  Read from  │────│  Extract    │────│   Decrypt   │────│   Vault     │
│    File     │    │   Nonce     │    │    with     │    │ Structure   │
│   (Disk)    │    │             │    │  ChaCha20   │    │   (RAM)     │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
```

### Save Vault Implementation

```c
int save_vault(const char *filepath, Vault *vault, const char *master_password) {
    VaultFileLayout layout;
    uint8_t key[MASTER_KEY_LEN];
    struct chacha20_context ctx;

    // 1. NONCE GENERATION - Critical for security
    int urandom_fd = open("/dev/urandom", O_RDONLY, 0);
    if (urandom_fd < 0) {
        puts("Error: Cannot open /dev/urandom.\n");
        return -1;
    }
    
    // Read 12 bytes of cryptographically secure random data
    if (read(urandom_fd, layout.nonce, CHACHA20_NONCE_LEN) != CHACHA20_NONCE_LEN) {
        puts("Error: Cannot read nonce from /dev/urandom.\n");
        close(urandom_fd);
        return -1;
    }
    close(urandom_fd);

    // 2. KEY PREPARATION
    normalize_key(master_password, key);  // Convert password to 32-byte key

    // 3. ENCRYPTION SETUP
    memcpy(layout.encrypted_vault, vault, sizeof(Vault)); // Copy vault data
    chacha20_init_context(&ctx, key, layout.nonce, 0);    // Initialize cipher
    chacha20_xor(&ctx, layout.encrypted_vault, sizeof(Vault)); // Encrypt in-place

    // 4. FILE WRITING
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0600); // Secure permissions
    if (fd < 0) {
        puts("Error: Cannot create vault file.\n");
        return -1;
    }
    
    ssize_t bytes_written = write(fd, &layout, sizeof(VaultFileLayout));
    close(fd);

    if (bytes_written != sizeof(VaultFileLayout)) {
        puts("Error writing to vault file.\n");
        return -1;
    }

    return 0;
}
```

**Key Security Measures:**
- **File permissions**: `0600` (read/write owner only)
- **Atomic operations**: Complete write or failure (no partial files)
- **Secure random**: `/dev/urandom` for nonce generation
- **Memory cleanup**: Could be improved with explicit zeroing

### Load Vault Implementation

```c
int load_vault(const char *filepath, Vault *vault, const char *master_password) {
    VaultFileLayout layout;
    uint8_t key[MASTER_KEY_LEN];
    struct chacha20_context ctx;

    // 1. FILE READING
    int fd = open(filepath, O_RDONLY, 0);
    if (fd < 0) return -1; // File doesn't exist or no permission
    
    ssize_t bytes_read = read(fd, &layout, sizeof(VaultFileLayout));
    close(fd);

    // 2. INTEGRITY CHECK
    if (bytes_read != sizeof(VaultFileLayout)) {
        puts("Error: Corrupted vault file or incorrect size.\n");
        return -1;
    }

    // 3. DECRYPTION
    normalize_key(master_password, key);
    chacha20_init_context(&ctx, key, layout.nonce, 0); // Same nonce from file
    chacha20_xor(&ctx, layout.encrypted_vault, sizeof(Vault)); // Decrypt in-place

    // 4. DATA EXTRACTION  
    memcpy(vault, layout.encrypted_vault, sizeof(Vault));
    
    return 0;
}
```

**Security Implications:**
- **Authentication**: Wrong password produces garbage data
- **No MAC**: File tampering detection relies on application logic
- **Side channels**: Timing attacks possible on password verification

---

## User Interface

### Command-Line Interface Design

PWMan implements a simple command-line interface with four main operations:

```bash
./pwman init <db_file>           # Initialize new vault
./pwman list <db_file>           # List all entries  
./pwman get <db_file> <entry>    # Retrieve specific password
./pwman add <db_file>            # Add new entry interactively
```

### Main Function Flow

```c
int main(int argc, char **argv) {
    // 1. ARGUMENT VALIDATION
    if (argc < 3) {
        print_usage();
        return 1;
    }

    const char *command = argv[1];
    const char *db_file = argv[2];

    // 2. INIT COMMAND (No master password needed)
    if (strcmp(command, "init") == 0) {
        if (argc != 3) { print_usage(); return 1; }
        return handle_init(db_file);
    }

    // 3. MASTER PASSWORD PROMPT (For all other commands)
    char master_pass[MAX_PASSWORD_LEN];
    printf("Please enter master password: ");
    if (readline(master_pass, MAX_PASSWORD_LEN) < 0) {
        return 1;
    }

    // 4. COMMAND DISPATCH
    if (strcmp(command, "list") == 0) {
        if (argc != 3) { print_usage(); return 1; }
        return handle_list(db_file, master_pass);
    }
    else if (strcmp(command, "get") == 0) {
        if (argc != 4) { print_usage(); return 1; }
        const char *entry_name = argv[3];
        return handle_get(db_file, entry_name, master_pass);
    }
    else if (strcmp(command, "add") == 0) {
        if (argc != 3) { print_usage(); return 1; }
        return handle_add(db_file, master_pass);
    }
    else {
        puts("Unknown command.\n");
        print_usage();
        return 1;
    }

    return 0;
}
```

### Command Implementations

#### Initialize Vault (`handle_init`)

```c
int handle_init(const char *db_file) {
    char pass1[MAX_PASSWORD_LEN], pass2[MAX_PASSWORD_LEN];

    // 1. PASSWORD COLLECTION
    printf("Creating vault '%s'\n", db_file);
    printf("Please enter a master password: ");
    if (readline(pass1, MAX_PASSWORD_LEN) < 0) return 1;

    printf("Confirm master password: ");
    if (readline(pass2, MAX_PASSWORD_LEN) < 0) return 1;

    // 2. PASSWORD VERIFICATION
    if (strcmp(pass1, pass2) != 0) {
        puts("Passwords do not match.\n");
        return 1;
    }

    // 3. EMPTY VAULT CREATION
    Vault vault;
    vault.count = 0;  // Start with empty vault
    
    // 4. SAVE ENCRYPTED VAULT
    if (save_vault(db_file, &vault, pass1) != 0) {
        puts("Error creating vault.\n");
        return 1;
    }

    puts("Vault created successfully!\n");
    return 0;
}
```

#### List Entries (`handle_list`)

```c
int handle_list(const char *db_file, const char* master_pass) {
    Vault vault;
    
    // 1. LOAD AND DECRYPT VAULT
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    // 2. DISPLAY ENTRIES
    if (vault.count == 0) {
        puts("Vault is empty.\n");
    } else {
        printf("Entries in vault (%d):\n", vault.count);
        for (int i = 0; i < vault.count; i++) {
            printf("- %s [%s] (%s)\n", 
                   vault.entries[i].name, 
                   vault.entries[i].platform, 
                   vault.entries[i].user);
        }
    }
    return 0;
}
```

#### Retrieve Entry (`handle_get`)

```c
int handle_get(const char *db_file, const char *entry_name, const char* master_pass) {
    Vault vault;
    
    // 1. LOAD VAULT
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    // 2. LINEAR SEARCH
    for (int i = 0; i < vault.count; i++) {
        if (strcmp(vault.entries[i].name, entry_name) == 0) {
            // 3. DISPLAY COMPLETE ENTRY (INCLUDING PASSWORD)
            printf("Entry: %s\n", vault.entries[i].name);
            printf("Platform: %s\n", vault.entries[i].platform);
            printf("Username: %s\n", vault.entries[i].user);
            printf("Password: %s\n", vault.entries[i].password);
            return 0;
        }
    }

    printf("Error: No entry found for '%s'.\n", entry_name);
    return 1;
}
```

#### Add Entry (`handle_add`)

```c
int handle_add(const char *db_file, const char* master_pass) {
    Vault vault;
    
    // 1. LOAD EXISTING VAULT
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    // 2. CAPACITY CHECK
    if (vault.count >= MAX_ENTRIES) {
        puts("Error: Vault is full.\n");
        return 1;
    }

    // 3. COLLECT ENTRY DATA
    char entry_name[MAX_NAME_LEN];
    char platform[MAX_PLATFORM_LEN];
    char user[MAX_USER_LEN];
    char pass1[MAX_PASSWORD_LEN], pass2[MAX_PASSWORD_LEN];

    printf("Entry name: ");
    if (readline(entry_name, MAX_NAME_LEN) < 0) return 1;

    // 4. DUPLICATE CHECK
    for (int i = 0; i < vault.count; i++) {
        if (strcmp(vault.entries[i].name, entry_name) == 0) {
            printf("Error: An entry named '%s' already exists.\n", entry_name);
            return 1;
        }
    }

    // 5. COLLECT REMAINING FIELDS
    printf("Platform: ");
    if (readline(platform, MAX_PLATFORM_LEN) < 0) return 1;

    printf("Username: ");
    if (readline(user, MAX_USER_LEN) < 0) return 1;

    printf("Password: ");
    if (readline(pass1, MAX_PASSWORD_LEN) < 0) return 1;
    printf("Confirm password: ");
    if (readline(pass2, MAX_PASSWORD_LEN) < 0) return 1;

    // 6. PASSWORD CONFIRMATION
    if (strcmp(pass1, pass2) != 0) {
        puts("Passwords do not match.\n");
        return 1;
    }

    // 7. ADD ENTRY TO VAULT
    memcpy(vault.entries[vault.count].name, entry_name, strlen(entry_name) + 1);
    memcpy(vault.entries[vault.count].platform, platform, strlen(platform) + 1);
    memcpy(vault.entries[vault.count].user, user, strlen(user) + 1);
    memcpy(vault.entries[vault.count].password, pass1, strlen(pass1) + 1);
    vault.count++;
    
    // 8. SAVE UPDATED VAULT
    if (save_vault(db_file, &vault, master_pass) != 0) {
        puts("Error saving vault.\n");
        return 1;
    }

    // 9. CONFIRMATION
    printf("Entry '%s' added successfully.\n", entry_name);
    printf("Platform: %s\n", platform);
    printf("Username: %s\n", user);
    return 0;
}
```

---

## Build System

### Makefile Analysis

The project uses a sophisticated Makefile that handles:
- **Custom libc compilation**: All functions compiled separately
- **Assembly integration**: NASM for runtime startup
- **Static linking**: Single executable with no dependencies
- **Custom entry point**: `_start` instead of standard `main`

```makefile
# Build configuration
CC = gcc
NASM = nasm
LD = ld

# Compilation flags
CFLAGS = -c -fno-stack-protector -I$(INCLUDE_DIR) -nostdlib -fno-builtin -Wall -Wextra
NASMFLAGS = -f elf64
LDFLAGS = -e _start

# Object files organization
LIBC_OBJS = $(LIBC_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/$(SRC_DIR)/%.o)
MAIN_OBJ = $(BUILD_DIR)/$(SRC_DIR)/main.o
CRYPTO_OBJ = $(BUILD_DIR)/$(SRC_DIR)/crypto.o
DATABASE_OBJ = $(BUILD_DIR)/$(SRC_DIR)/database.o
ASM_OBJS = $(BUILD_DIR)/crt0.o

# Final linking
$(NAME): $(PWMAN_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
```

### Compilation Process

```
Source Files                Object Files               Final Binary
┌─────────────┐            ┌─────────────┐           ┌─────────────┐
│ crt0.asm    │──nasm────→ │ crt0.o      │           │             │
├─────────────┤            ├─────────────┤           │             │
│ main.c      │──gcc─────→ │ main.o      │           │             │
├─────────────┤            ├─────────────┤    ld     │             │
│ crypto.c    │──gcc─────→ │ crypto.o    │ ────────→ │   pwman     │
├─────────────┤            ├─────────────┤           │ (executable)│
│ database.c  │──gcc─────→ │ database.o  │           │             │
├─────────────┤            ├─────────────┤           │             │
│ libc/*.c    │──gcc─────→ │ libc/*.o    │           │             │
└─────────────┘            └─────────────┘           └─────────────┘
```

**Key Build Features:**
- **No standard library**: `-nostdlib` prevents linking with glibc
- **No built-ins**: `-fno-builtin` disables GCC optimizations that assume standard library
- **Stack protection off**: `-fno-stack-protector` for minimal binary
- **Custom entry**: `-e _start` uses assembly entry point
- **Static linking**: All code included in single executable

---

## Security Analysis

### Strengths

**Cryptographic Security:**
- **Modern cipher**: ChaCha20 is cryptographically secure
- **Fresh nonces**: Each save uses new random nonce from `/dev/urandom`
- **Key stretching**: Master password normalized to full 32-byte key
- **No key reuse**: Different nonces ensure unique ciphertexts

**System Security:**
- **Minimal attack surface**: No external dependencies
- **File permissions**: Vault files created with restrictive `0600` permissions
- **No network code**: Local-only operation reduces attack vectors
- **Memory isolation**: Custom memory management avoids libc vulnerabilities

### Weaknesses and Vulnerabilities

**Password Security:**
- **No key derivation**: Direct password-to-key mapping vulnerable to dictionary attacks
- **No salt**: Same password always produces same key
- **Plaintext in memory**: Master password stored unencrypted in RAM
- **No secure input**: Passwords visible in process memory

**Authentication:**
- **No MAC**: File tampering not detected until application logic fails
- **Weak authentication**: Wrong password produces garbage, not explicit failure
- **No integrity checking**: Corrupted entries might go unnoticed

**Implementation Issues:**
- **Memory leaks potential**: No explicit memory cleanup for sensitive data
- **Buffer overflows**: Fixed-size buffers could overflow with long inputs
- **Side-channel attacks**: Timing attacks possible on password comparison
- **Error handling**: Some edge cases not properly handled

**Recommended Improvements:**
1. **PBKDF2/Argon2**: Proper key derivation function
2. **HMAC authentication**: File integrity verification  
3. **Secure memory**: `mlock()` and explicit zeroing
4. **Input validation**: Length checks on all user inputs
5. **Constant-time comparison**: Prevent timing attacks

---

## Flow Diagrams

### Application Flow

```
Program Start
     │
     ▼
┌─────────────────┐
│ Parse Arguments │
└─────────────────┘
     │
     ▼
┌─────────────────┐    Yes    ┌─────────────────┐
│ Command = init? │ ────────→ │  handle_init()  │
└─────────────────┘           └─────────────────┘
     │ No                            │
     ▼                               ▼
┌─────────────────┐           ┌─────────────────┐
│ Get Master Pass │           │ Create New Vault│
└─────────────────┘           └─────────────────┘
     │                               │
     ▼                               ▼
┌─────────────────┐           ┌─────────────────┐
│ Route Command:  │           │ Encrypt & Save  │
│ • list          │           └─────────────────┘
│ • get           │                     │
│ • add           │                     ▼
└─────────────────┘              ┌─────────────────┐
     │                           │ Success/Failure │
     ▼                           └─────────────────┘
┌─────────────────┐
│ Load & Decrypt  │
│ Vault           │
└─────────────────┘
     │
     ▼
┌─────────────────┐
│ Execute Command │
└─────────────────┘
     │
     ▼
┌─────────────────┐
│ Save Changes    │
│ (if needed)     │
└─────────────────┘
     │
     ▼
┌─────────────────┐
│ Exit            │
└─────────────────┘
```

### Encryption/Decryption Flow

```
Save Operation:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Vault in Memory │ →  │ Generate Random │ →  │ Normalize Key   │
│ (Plaintext)     │    │ Nonce (12 bytes)│    │ from Password   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Copy to Layout  │ ←  │ Initialize      │ ←  │ 32-byte Key     │
│ Buffer          │    │ ChaCha20 Context│    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │
         ▼                       ▼
┌─────────────────┐    ┌─────────────────┐
│ Encrypt in-place│ →  │ Write to File:  │
│ with ChaCha20   │    │ Nonce + Data    │
└─────────────────┘    └─────────────────┘

Load Operation:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Read File:      │ →  │ Extract Nonce   │ →  │ Normalize Key   │
│ Nonce + Data    │    │ (first 12 bytes)│    │ from Password   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Verify Size     │ ←  │ Initialize      │ ←  │ 32-byte Key     │
│ (25,616 bytes)  │    │ ChaCha20 Context│    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │
         ▼                       ▼
┌─────────────────┐    ┌─────────────────┐
│ Decrypt in-place│ →  │ Copy to Vault   │
│ with ChaCha20   │    │ Structure       │
└─────────────────┘    └─────────────────┘
```

### ChaCha20 Block Generation

```
Initial State (16 words):
┌─────┬─────┬─────┬─────┐
│  0  │  1  │  2  │  3  │ ← Constants
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  7  │ ← Key (part 1)
├─────┼─────┼─────┼─────┤
│  8  │  9  │ 10  │ 11  │ ← Key (part 2)
├─────┼─────┼─────┼─────┤
│ 12  │ 13  │ 14  │ 15  │ ← Counter + Nonce
└─────┴─────┴─────┴─────┘
         │
         ▼ Copy to Working State
┌─────────────────────────┐
│ Apply 20 Rounds         │
│ (10 × Double Round)     │
│                         │
│ Each Round:             │
│ • 4 Column Operations   │
│ • 4 Diagonal Operations │
└─────────────────────────┘
         │
         ▼ Add Original State
┌─────────────────────────┐
│ 64-byte Keystream Block │
└─────────────────────────┘
         │
         ▼ XOR with Data
┌─────────────────────────┐
│ Encrypted/Decrypted     │
│ Data Block              │
└─────────────────────────┘
```

---

## Conclusion

PWMan demonstrates a complete, minimal password manager implementation using low-level C programming concepts. The project showcases:

- **System programming**: Direct system calls without standard library
- **Cryptographic implementation**: Modern ChaCha20 stream cipher
- **File format design**: Binary storage with encryption
- **Memory management**: Custom heap implementation
- **Build system**: Complex Makefile with assembly integration

While functional for educational purposes, production use would require significant security enhancements, particularly around key derivation, authentication, and memory protection.

The codebase serves as an excellent example of how modern encryption can be implemented at a low level while maintaining good code organization and clear separation of concerns.