SECTION .text
    global _start
 
_start:
    ; Retrieve argc, argv, envp from stack
    mov     rdi, [rsp]             ; argc
    lea     rsi, [rsp + 8]         ; argv
    lea     rdx, [rsi + rdi*8 + 8] ; envp
 
    ; call main(argc, argv, envp)
    extern main
    call    main
 
    ; system call exit(ret)
    mov     rdi, rax            ; main returned value
    mov     rax, 60             ; syscall exit
    syscall