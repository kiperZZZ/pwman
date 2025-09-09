# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **pwman**, a simple password manager implemented in C for Linux x86_64. The project is built without standard libraries (`-nostdlib -fno-builtin`) and uses direct system calls for all operations. It reuses the custom libc implementation from the 0x41sh project.

## Project Requirements

### Core Features to Implement

1. **Master Password Authentication**
   - Prompt for master password to decrypt database
   - Secure input (masked password entry)
   - Password verification against stored hash

2. **Database Management**
   - List-based structure storing entries
   - Each entry contains: name (service/website) + password
   - Encrypted storage on disk
   - Load/save functionality

3. **Command-Line Interface**
   Choose one of these approaches:
   ```bash
   # CLI approach (recommended)
   ./pwman init vault.db              # Initialize new vault
   ./pwman add vault.db github.com    # Add new entry
   ./pwman list vault.db              # List all entries  
   ./pwman get vault.db github.com    # Retrieve password
   
   # OR Interactive approach
   ./pwman vault.db                   # Start interactive session
   ```

4. **Encryption Implementation**
   - Custom encryption algorithm (no external crypto libraries)
   - Simple substitution cipher is acceptable
   - XOR encryption already started in `src/crypto.c`

## Technical Architecture

### Build System
- **Makefile**: Adapted from 0x41sh project
- **Compilation**: `gcc -nostdlib -fno-builtin`  
- **Linking**: `ld -e _start` (custom entry point)
- **No standard library dependencies**

### Project Structure
```
pwman/
├── src/
│   ├── main.c          # Main program and CLI parsing
│   ├── crypto.c        # Encryption/decryption functions
│   ├── database.c      # Database operations (add/list/get/save/load)
│   ├── strings.c       # Additional string functions
│   └── libc/           # Custom libc (reused from 0x41sh)
├── include/libc/       # Header files
├── crt0.asm           # Assembly startup code
└── Makefile           # Build configuration
```

### Available Functions from libc

**Memory Management**:
- `malloc()`, `free()`, `realloc()` - Memory allocation
- `sbrk()`, `brk()` - Heap management

**I/O Operations**:
- `read()`, `write()` - File I/O
- `printf()`, `puts()`, `putchar()` - Output
- `getline()` - Line input

**String Operations**:
- `strcmp()`, `strncmp()` - String comparison
- `strlen()`, `strcpy()`, `strcat()` - Added in strings.c

**Process Control**:
- `exit()` - Program termination
- `fork()`, `execve()`, `waitpid()` - Process management

## Implementation Tasks

### Phase 1: CLI and Argument Parsing
```c
// In main.c - implement command parsing
int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        exit(1);
    }
    
    if (strcmp(argv[1], "init") == 0) {
        return cmd_init(argv[2]);
    } else if (strcmp(argv[1], "add") == 0) {
        return cmd_add(argv[2], argv[3]);
    }
    // ... other commands
}
```

### Phase 2: Database Structure
```c
// In database.c - define data structures
typedef struct password_entry {
    char name[256];         // Service/website name
    char password[256];     // Associated password
    struct password_entry *next;  // Linked list
} password_entry_t;

typedef struct vault {
    password_entry_t *entries;    // Head of linked list
    char master_hash[64];         // Hashed master password
    int entry_count;              // Number of entries
} vault_t;
```

### Phase 3: File Operations
```c
// Database persistence using direct syscalls
int save_vault(vault_t *vault, const char *filename);
int load_vault(vault_t *vault, const char *filename);

// Use these libc functions:
// - File descriptor operations with read()/write()
// - Memory management with malloc()/free()
```

### Phase 4: Encryption
```c
// In crypto.c - implement encryption
void encrypt_entry(password_entry_t *entry, char key);
void decrypt_entry(password_entry_t *entry, char key);
void encrypt_vault_data(char *data, size_t size, char key);

// Simple XOR already implemented:
void simple_encrypt(char *data, char key);
void simple_decrypt(char *data, char key);
```

### Phase 5: Secure Input
```c
// Implement masked password input
int get_master_password(char *buffer, size_t size);
// Use read() with terminal control to hide input
```

## Development Guidelines

### Security Considerations
- **No plaintext storage**: Always encrypt passwords on disk
- **Memory cleanup**: Clear sensitive data after use
- **Input validation**: Check buffer sizes to prevent overflows
- **Master password hashing**: Don't store plaintext master password

### Error Handling
- **System call failures**: Check return values of read()/write()
- **Memory allocation**: Check malloc() return values
- **File operations**: Handle file not found, permissions, etc.
- **User input**: Validate command arguments

### Code Style
- Follow existing libc code patterns
- Use direct system calls consistently
- Minimal error handling (educational focus)
- Clear, readable function names

## Testing Strategy

### Manual Testing Commands
```bash
# Test basic functionality
make clean && make

# Test database initialization
./pwman init test_vault.db

# Test adding entries
./pwman add test_vault.db github.com

# Test listing entries  
./pwman list test_vault.db

# Test password retrieval
./pwman get test_vault.db github.com
```

### File System Testing
- Test with different file permissions
- Test with non-existent files
- Test with corrupted vault files
- Test disk space limitations

## Bonus Features (Optional)

1. **Improved Encryption**: Implement Caesar cipher or custom substitution
2. **Password Generator**: Generate secure random passwords
3. **Search Functionality**: Find entries by partial name match
4. **Import/Export**: Support for different file formats
5. **Backup Creation**: Automatic vault backups
6. **Entry Metadata**: Creation date, last modified, etc.

## Constraints and Limitations

### Technical Constraints
- **No standard library**: Must use custom libc only
- **Direct syscalls**: All OS interaction through system calls
- **Static linking**: Single executable with no external dependencies
- **x86_64 Linux**: Platform-specific implementation

### Educational Focus
- **Learning objective**: Understanding system programming
- **Simplicity over security**: Basic encryption is acceptable
- **Code clarity**: Readable implementation preferred over optimization

## Deliverables

### Code Structure
- **Clean compilation**: No warnings or errors
- **Working executable**: All commands functional
- **Proper error handling**: Graceful failure modes
- **Code documentation**: Clear comments where needed

### Demonstration
- **Live demo**: Show all features working
- **Problem explanation**: Discuss challenges faced
- **Solution presentation**: Explain implementation choices
- **Creative features**: Highlight any bonus implementations

## Development Notes

- **Reuse existing code**: Leverage 0x41sh libc implementation
- **Incremental development**: Implement one feature at a time
- **Regular testing**: Test after each major change
- **Keep it simple**: Focus on core requirements first
- **Document decisions**: Explain design choices in comments