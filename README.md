# PWMan - Password Manager in C

A minimalist password manager developed entirely in C for Linux x86_64, using only system calls without dependencies on standard libraries.

## Features

- **No standard libraries**: Compiled with `-nostdlib -fno-builtin`
- **Simple and intuitive command-line interface**
- **XOR encryption** to secure stored data
- **Vault management** with master password
- **Custom libc implementation** reused from the 0x41sh project
- **Direct system calls** for all operations

## Prerequisites

- Linux x86_64
- GCC (GNU Compiler Collection)
- NASM (Netwide Assembler)

## Installation

```bash
git clone https://github.com/kiperZZZ/pwman.git
cd pwman
make
```

## Usage

### Initialize a new vault
```bash
./pwman init vault.db
```

### List all entries
```bash
./pwman list vault.db
```

### Add a new entry
```bash
./pwman add vault.db
```

### Retrieve a password
```bash
./pwman get vault.db github.com
```

## Architecture

```
pwman/
├── src/
│   ├── main.c          # Main program and CLI parsing
│   ├── crypto.c        # Encryption/decryption functions
│   ├── database.c      # Database operations (CRUD)
│   └── libc/           # Custom libc implementation
├── include/
│   ├── libc/           # Header files
│   └── pwman.h         # Main definitions
├── crt0.asm           # Assembly startup code
└── Makefile           # Build configuration
```

## Technical Features

### Memory Management
- `malloc()`, `free()`, `realloc()` - Memory allocation
- `sbrk()`, `brk()` - Heap management

### I/O Operations
- `read()`, `write()` - File I/O
- `printf()`, `puts()`, `putchar()` - Output
- `getline()` - Line input

### String Operations
- `strcmp()`, `strncmp()` - String comparison
- `strlen()`, `strcpy()`, `strcat()` - String manipulation

## Security

- **No plaintext storage**: All passwords are encrypted
- **Master password hashing**: Master password is never stored in plaintext
- **Memory cleanup**: Sensitive data is cleared after use
- **Input validation**: Buffer size checks to prevent overflows

## Testing

```bash
# Test basic functionality
make clean && make

# Test initialization
./pwman init test_vault.db

# Test adding entries
./pwman add test_vault.db

# Test listing entries
./pwman list test_vault.db

# Test password retrieval
./pwman get test_vault.db example.com
```

## Educational Purpose

This project was developed for educational purposes to understand:
- System programming in C
- Linux system calls
- Custom libc implementation
- Compilation without standard libraries
- Low-level memory management

## Limitations

- **Platform specific**: Linux x86_64 only
- **Basic encryption**: Simple XOR (for educational purposes)
- **No networking**: Local storage only
- **Limited features**: Minimalist password manager

## Contributing

Contributions are welcome! Feel free to open an issue or submit a pull request.

## License

This project is developed for educational purposes. See the LICENSE file for more details.

---

**Note**: This password manager uses simple encryption for educational purposes. For production use, employ more robust encryption solutions.