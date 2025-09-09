#include "pwman.h"

static void print_usage() {
    puts("Usage:\n");
    puts("  ./pwman init <db_file>           # Initialize a new vault\n");
    puts("  ./pwman list <db_file>           # List all entries\n");
    puts("  ./pwman get <db_file> <entry>    # Retrieve a password\n");
    puts("  ./pwman add <db_file>            # Add a new entry\n");
}

static ssize_t read_line(char *buf, size_t size) {
    ssize_t i = 0;
    while (i < size - 1) {
        ssize_t ret = read(0, &buf[i], 1);
        if (ret <= 0) return -1;
        if (buf[i] == '\n') {
            buf[i] = '\0';
            return i;
        }
        i++;
    }
    buf[i] = '\0';
    return i;
}

int handle_init(const char *db_file) {
    char pass1[MAX_PASSWORD_LEN], pass2[MAX_PASSWORD_LEN];

    printf("Creating vault '%s'\n", db_file);
    printf("Please enter a master password: ");
    if (read_line(pass1, MAX_PASSWORD_LEN) < 0) return 1;

    printf("Confirm master password: ");
    if (read_line(pass2, MAX_PASSWORD_LEN) < 0) return 1;

    if (strcmp(pass1, pass2) != 0) {
        puts("Passwords do not match.\n");
        return 1;
    }

    Vault vault;
    vault.count = 0;
    
    if (save_vault(db_file, &vault, pass1) != 0) {
        puts("Error creating vault.\n");
        return 1;
    }

    puts("Vault created successfully!\n");
    return 0;
}

int handle_list(const char *db_file, const char* master_pass) {
    Vault vault;
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    if (vault.count == 0) {
        puts("Vault is empty.\n");
    } else {
        printf("Entries in vault (%d):\n", vault.count);
        for (int i = 0; i < vault.count; i++) {
            printf("- %s [%s] (%s)\n", vault.entries[i].name, vault.entries[i].platform, vault.entries[i].user);
        }
    }
    return 0;
}

int handle_get(const char *db_file, const char *entry_name, const char* master_pass) {
    Vault vault;
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    for (int i = 0; i < vault.count; i++) {
        if (strcmp(vault.entries[i].name, entry_name) == 0) {
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

int handle_add(const char *db_file, const char* master_pass) {
    Vault vault;
    if (load_vault(db_file, &vault, master_pass) != 0) {
        puts("Incorrect password or corrupted file.\n");
        return 1;
    }

    if (vault.count >= MAX_ENTRIES) {
        puts("Error: Vault is full.\n");
        return 1;
    }

    char entry_name[MAX_NAME_LEN];
    char platform[MAX_PLATFORM_LEN];
    char user[MAX_USER_LEN];
    char pass1[MAX_PASSWORD_LEN], pass2[MAX_PASSWORD_LEN];

    printf("Entry name: ");
    if (read_line(entry_name, MAX_NAME_LEN) < 0) return 1;

    for (int i = 0; i < vault.count; i++) {
        if (strcmp(vault.entries[i].name, entry_name) == 0) {
            printf("Error: An entry named '%s' already exists.\n", entry_name);
            return 1;
        }
    }

    printf("Platform: ");
    if (read_line(platform, MAX_PLATFORM_LEN) < 0) return 1;

    printf("Username: ");
    if (read_line(user, MAX_USER_LEN) < 0) return 1;

    printf("Password: ");
    if (read_line(pass1, MAX_PASSWORD_LEN) < 0) return 1;
    printf("Confirm password: ");
    if (read_line(pass2, MAX_PASSWORD_LEN) < 0) return 1;

    if (strcmp(pass1, pass2) != 0) {
        puts("Passwords do not match.\n");
        return 1;
    }

    memcpy(vault.entries[vault.count].name, entry_name, strlen(entry_name) + 1);
    memcpy(vault.entries[vault.count].platform, platform, strlen(platform) + 1);
    memcpy(vault.entries[vault.count].user, user, strlen(user) + 1);
    memcpy(vault.entries[vault.count].password, pass1, strlen(pass1) + 1);
    vault.count++;
    
    if (save_vault(db_file, &vault, master_pass) != 0) {
        puts("Error saving vault.\n");
        return 1;
    }

    printf("Entry '%s' added successfully.\n", entry_name);
    printf("Platform: %s\n", platform);
    printf("Username: %s\n", user);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    const char *command = argv[1];
    const char *db_file = argv[2];

    if (strcmp(command, "init") == 0) {
        if (argc != 3) { print_usage(); return 1; }
        return handle_init(db_file);
    }

    char master_pass[MAX_PASSWORD_LEN];
    printf("Please enter master password: ");
    if (read_line(master_pass, MAX_PASSWORD_LEN) < 0) {
        return 1;
    }

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