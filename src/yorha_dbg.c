#include "../include/yorha_dbg.h"


#define DBG_HANDLE_LOOP while (yorha_handle_command(conn)) continue

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

    if (!current_command || current_command->header.command_type > __max_dbg_trap_handlers) return YORHA_FAILURE;

    // command_trap_handler trap_handler;
    int status = YORHA_SUCCESS;

    kprintf("Handling trap frame...\n");
    //
    // Command handler
    //
    switch (current_command->header.command_type)
    {
        case PAUSE_KERNEL:
            kprintf("trap_frame: handling with pause_kernel_trap_handler\n");
            status = pause_kernel_trap_handler(current_command, current_connection, ctx);
            // DBG_HANDLE_LOOP();
        default:
            kprintf("Unhandled command %d\n", current_command->header.command_type);  
    }


    kprintf("Resuming execution...\n");
    
    return status; // useless
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
        //
        // Debugger loop
        //
        while (dbg_status == RUNNING)
        {
            conn = kaccept(sock, NULL, NULL, td);
            if (conn < 0)
            {
                kprintf("Error handling connection...\n");
                break;
            }
            
        read_data:
        /*
            while (yorha_dbg_handle_command(conn) != CLIENT_CLOSED) continue;
        */
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

        switch (command->header.command_type)
        {
            case PAUSE_KERNEL:
                return pause_kernel_executor(command, conn);
            default:
                kprintf("Unhandled command %d\n", command->header.command_type);
        }
        // command_executor executor = command_executor_handlers[command->header.command_type];
        // return executor(command, conn);
    }

    kprintf("Invalid command received %d\n", command->header.command_type);

    return YORHA_SUCCESS;
}

//
// Launch the int3 in a separated kthread
//
int pause_kernel_executor(dbg_command*, int)
{
    kprintf("pause_kernel_executor\n");
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
    memset(response.code, 0x00, PAUSE_KERNEL_CODE_DUMP_SIZE);
    
    response.header.command_type = PAUSE_KERNEL;
    response.header.response_size = sizeof(response);

    int res = ksendto(current_connection, &response, sizeof(response), 0, 0, 0, main_thread);
    if (res < 0)
    {
        kprintf("Error calling ksendto!\n");
        return YORHA_FAILURE;
    }

    /*
    while (yorha_handle_command(conn)) continue;
    */
    return YORHA_SUCCESS;
}


int stop_debugger_executor(dbg_command*, int)
{
    kprintf("Exiting debugger...\n");
    dbg_status = STOPPED;

    return YORHA_SUCCESS;
}


