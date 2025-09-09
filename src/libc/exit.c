#include "libc/libc.h" 

void exit(int status) {
    __asm__ volatile (
        "mov $60, %%rax\n"     // syscall: exit
        "mov %0, %%rdi\n"      // status
        "syscall\n"
        :
        : "r"((long)status)
        : "rax", "rdi"
    );
    
    // Cette ligne ne devrait jamais être atteinte
    __builtin_unreachable();
}