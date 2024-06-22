#include "../include/yorha_dbg.h"


void yorha_dbg_breakpoint_handler(X64Registers* registers)
{
    if (!kprintf)
    {
        // Should never be called, but who knows
        init_kernel();
    }

    kprintf("RAX: 0x%llx\n", registers->rax);
    kprintf("RCX: 0x%llx\n", registers->rcx);
    kprintf("RDX: 0x%llx\n", registers->rdx);
    kprintf("RBP: 0x%llx\n", registers->rbp);
    kprintf("RSI: 0x%llx\n", registers->rsi);
    kprintf("RDI: 0x%llx\n", registers->rdi);
    kprintf("R8: 0x%llx\n", registers->r8);
    kprintf("R9: 0x%llx\n", registers->r9);
    kprintf("R10: 0x%llx\n", registers->r10);
    kprintf("R11: 0x%llx\n", registers->r11);
    kprintf("R12: 0x%llx\n", registers->r12);
    kprintf("R13: 0x%llx\n", registers->r13);
    kprintf("R14: 0x%llx\n", registers->r14);
    kprintf("R15: 0x%llx\n", registers->r15);
}