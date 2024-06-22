.intel_syntax noprefix
.global int_breakpoint_handler
.extern yorha_dbg_breakpoint_handler

.macro SAVE_GP
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
.endm

.macro RESTORE_GP
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
.endm

.text


int_breakpoint_handler:
    SAVE_GP
    mov rdi, rsp
    sub rsp, 0x20
    call yorha_dbg_breakpoint_handler
    RESTORE_GP
    iretq



