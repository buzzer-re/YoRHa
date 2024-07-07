#include "../include/yorha_dbg.h"

int sock = -1;
enum DbgStatus dbg_status = IDLE;
struct sockaddr_in sockaddr;
int current_connection;
struct thread* main_thread;
dbg_command* current_command = NULL;

int yorha_dbg_breakpoint_handler(trap_frame_ctx* ctx)
{
    if (!kprintf)
    {
        // Should never be called, but who knows
        init_kernel();
    }

    if (!current_command || current_command->header.command_type < __max_dbg_trap_handlers) return YORHA_FAILURE;

    command_trap_handler trap_handler;

    //
    // Command handler
    //
    switch (current_command->header.command_type)
    {
        case STOP_DBG:
            trap_handler = command_trap_handlers[current_command->header.command_type];
            return trap_handler(current_command, current_connection, ctx);
        default:
            return YORHA_SUCCESS;       
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

    // kprintf("YorhaDBG handler called, dumping registers...\n");
    // kprintf("RAX: 0x%llx\n", ctx->rax);
    // kprintf("RCX: 0x%llx\n", ctx->rcx);
    // kprintf("RDX: 0x%llx\n", ctx->rdx);
    // kprintf("RBP: 0x%llx\n", ctx->rbp);
    // kprintf("RSI: 0x%llx\n", ctx->rsi);
    // kprintf("RDI: 0x%llx\n", ctx->rdi);
    // kprintf("R8: 0x%llx\n", ctx->r8);
    // kprintf("R9: 0x%llx\n", ctx->r9);
    // kprintf("R10: 0x%llx\n", ctx->r10);
    // kprintf("R11: 0x%llx\n", ctx->r11);
    // kprintf("R12: 0x%llx\n", ctx->r12);
    // kprintf("R13: 0x%llx\n", ctx->r13);
    // kprintf("R14: 0x%llx\n", ctx->r14);
    // kprintf("R15: 0x%llx\n", ctx->r15);
    // kprintf("RIP: 0x%llx\n", ctx->rip);
    // kprintf("CS: 0x%llx\n", ctx->cs);
    // kprintf("EFLAGS: 0x%llx\n", ctx->eflags);
    // kprintf("RSP: 0x%llx\n", ctx->rsp);
    // kprintf("SS: 0x%llx\n", ctx->ss);

    kprintf("Resuming execution...\n");
    // dont know about that

    return YORHA_SUCCESS; // useless
}


//
// Initialize the network debugger server
//
int yorha_dbg_run_debug_server_loop(int port)
{
    if (!kernel_base)
    {
        return YORHA_FAILURE;
    }
    
    uint8_t command_data[0x1000];
    size_t cmd_size;
    int conn;
    struct thread* td = curthread;

    sock = ksocket(AF_INET, SOCK_STREAM, 0, td);

    if (sock < 0)
    {
        kprintf("Unable to create socket!\n");
        return YORHA_FAILURE;
    } 

    
    struct sockaddr_in sockaddr;
    socklen_t socklen = sizeof(sockaddr);
    sockaddr.sin_len = socklen;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = __builtin_bswap16(port);
    sockaddr.sin_addr.s_addr  = __builtin_bswap32(INADDR_ANY);
    
    if (kbind(sock, (struct sockaddr*) &sockaddr, socklen, td) < 0)
    {
        kprintf("Unable to bind socket %d on port %d\n", sock, port);
        kclose(sock, td);
        return YORHA_FAILURE;
    }


    if (klisten(sock, 4, td) < 0)
    {
        kprintf("Unable to listen socket %d on port %d\n",sock, port);
        kclose(sock, td);
        return YORHA_FAILURE;
    }

    dbg_status = RUNNING;    
    if (sock >= 0)
    {
        while (dbg_status == RUNNING)
        {
            conn = kaccept(sock, NULL, NULL, td);
            if (conn < 0)
            {
                kprintf("Error handling connection...\n");
                break;
            }
            
        read_data:
            if ((cmd_size = kread(conn, command_data, 0x1000, td)) > 0)
            {
                dbg_command* command = (dbg_command*) command_data;

                if (yorha_dbg_handle_command(command, conn) != STOP_DBG)
                {
                    kprintf("Waiting next commands...\n");
                    goto read_data;
                }

                kprintf("Received STOP_DBG command from remote, closing connection...\n");
            }
            
            kclose(conn, td);
        } 

        kclose(sock, td);
    }

    return YORHA_SUCCESS;
}

int yorha_dbg_handle_command(dbg_command* command, int conn)
{
    if (command->header.command_type < __max_dbg_commands )
    {
        //
        // Save the current thread pointer and connection, this will be used in the int3 handler
        //
        kprintf("Received command %d\n", command->header.command_type);
        main_thread = curthread;
        current_connection = conn;
        current_command = command;
        command_executor executor = command_executor_handlers[command->header.command_type];
        return executor(command, conn);
    }

    kprintf("Invalid command received %d\n", command->header.command_type);
    // ksend(conn, "invalid command", )
    return YORHA_SUCCESS;
}

//
// Launch the int3 in a separated kthread
//
int pause_kernel_executor(dbg_command*, int)
{
    kproc_create(__debugbreak, 0, 0, 0, 0, "__debugbreak");
    return YORHA_SUCCESS;
}

int pause_kernel_trap_handler(dbg_command*, int, trap_frame_ctx* ctx)
{
    //
    // Reply the current register state and instruction pointer
    // 
    kprintf("pause_kernel_trap_handler called!\n");
    pause_kernel_response_data_t response = {0};

    memcpy(&response.regs, &ctx->regs, sizeof(registers_t));
    memset(response.code, 0x00, 0x64);
    
    response.header.command_type = STOP_DBG;
    response.header.response_size = sizeof(response);

    ksendto(current_connection, &response, sizeof(response), 0, 0, 0, main_thread);

    return YORHA_SUCCESS;
}


int stop_debugger_executor(dbg_command*, int)
{
    kprintf("Exiting debugger...\n");
    dbg_status = STOPPED;

    return YORHA_SUCCESS;
}


