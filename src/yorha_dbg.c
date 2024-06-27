#include "../include/yorha_dbg.h"

int sock;
struct sockaddr_in sockaddr;

void yorha_dbg_breakpoint_handler(trap_frame_ctx* ctx)
{
    if (!kprintf)
    {
        // Should never be called, but who knows
        init_kernel();
    }

    //
    // Build DebugStruct packet, check if any debugee is connected and send the info
    // Wait for connections
    // Handle commands
    // Resume execution if `c/g/continue` is send, step if `nexti,stepi,n` is sent
    // disas <num> -> read <num> byte from the RIP
    // dd <num> read N uint32_t from a given address
    // dq <num> read N uint64_t from a given address
    // db <num> read N uint8_t from a given address
    // db/bp <addr> place a breakpoint `int3` at the given address
    //

    kprintf("YorhaDBG handler called, dumping registers...\n");
    kprintf("RAX: 0x%llx\n", ctx->rax);
    kprintf("RCX: 0x%llx\n", ctx->rcx);
    kprintf("RDX: 0x%llx\n", ctx->rdx);
    kprintf("RBP: 0x%llx\n", ctx->rbp);
    kprintf("RSI: 0x%llx\n", ctx->rsi);
    kprintf("RDI: 0x%llx\n", ctx->rdi);
    kprintf("R8: 0x%llx\n", ctx->r8);
    kprintf("R9: 0x%llx\n", ctx->r9);
    kprintf("R10: 0x%llx\n", ctx->r10);
    kprintf("R11: 0x%llx\n", ctx->r11);
    kprintf("R12: 0x%llx\n", ctx->r12);
    kprintf("R13: 0x%llx\n", ctx->r13);
    kprintf("R14: 0x%llx\n", ctx->r14);
    kprintf("R15: 0x%llx\n", ctx->r15);
    kprintf("RIP: 0x%llx\n", ctx->rip);
    kprintf("CS: 0x%llx\n", ctx->cs);
    kprintf("EFLAGS: 0x%llx\n", ctx->eflags);
    kprintf("RSP: 0x%llx\n", ctx->rsp);
    kprintf("SS: 0x%llx\n", ctx->ss);

    kprintf("Resuming execution...\n");
    // dont know about that
}


int yorha_dbg_init_debug_server(int port)
{
    if (!kernel_base)
    {
        return YORHA_FAILURE;
    }

    kprintf("Starting debug server, creating socket...\n");



    sock = ksocket(AF_INET, SOCK_STREAM, 0, curthread);
    
    if (sock < 0)
    {
        kprintf("Unable to create socket!\n");
        return YORHA_FAILURE;
    } 

    kprintf("Socket created: %d\n", sock);
    
    struct sockaddr_in sockaddr = {0};
    sockaddr.sin_len = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = __builtin_bswap16(port);
    sockaddr.sin_addr.s_addr  = __builtin_bswap32(INADDR_ANY);
    
    // if (!kbind(soc, (struct sockaddr) sockaddr);

    kclose(sock, curthread);

    
    return YORHA_SUCCESS;
}