NAME = pwman
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

LIBC_SRCS = $(wildcard $(SRC_DIR)/libc/*.c)
MAIN_SRC = $(SRC_DIR)/main.c
CRYPTO_SRC = $(SRC_DIR)/crypto.c
DATABASE_SRC = $(SRC_DIR)/database.c

LIBC_OBJS = $(LIBC_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/$(SRC_DIR)/%.o)
MAIN_OBJ = $(BUILD_DIR)/$(SRC_DIR)/main.o
CRYPTO_OBJ = $(BUILD_DIR)/$(SRC_DIR)/crypto.o
DATABASE_OBJ = $(BUILD_DIR)/$(SRC_DIR)/database.o
ASM_OBJS = $(BUILD_DIR)/crt0.o

PWMAN_OBJS = $(ASM_OBJS) $(LIBC_OBJS) $(MAIN_OBJ) $(CRYPTO_OBJ) $(DATABASE_OBJ)

CC = gcc
NASM = nasm
LD = ld

CFLAGS = -c -fno-stack-protector -I$(INCLUDE_DIR) -nostdlib -fno-builtin -Wall -Wextra
NASMFLAGS = -f elf64
LDFLAGS = -e _start

all: $(NAME)

$(NAME): $(PWMAN_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
