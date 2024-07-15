#include "../include/yorha_dbg.h"


#define DBG_HANDLE_LOOP while (yorha_handle_command(conn)) continue

int sock = -1;
enum DbgStatus dbg_status = IDLE;
struct sockaddr_in sockaddr;
int current_connection;
struct thread* main_thread;
dbg_command* current_command = NULL;

int yorha_dbg_breakpoint_handler(trap_frame_t* ctx)
{
    if (!kprintf)
    {
        // Should never be called, but who knows
        init_kernel();
    }

    if (!current_command || current_command->header.command_type > __max_dbg_trap_handlers) return YORHA_FAILURE;

    // command_trap_handler trap_handler;
    int status = YORHA_SUCCESS;

    status = yorha_trap_command_handler(ctx);

    while(1) continue;

    kprintf("Resuming execution...\n");
    
    return status;
}

int yorha_trap_command_handler(trap_frame_t* ctx)
{
    //
    // Command handler
    //
    uint8_t command_data[0x1000];
    int status = YORHA_SUCCESS;
    while (1)
    {
        kprintf("Processing command %d\n", current_command->header.command_type);
        switch (current_command->header.command_type)
        {
            case PAUSE_KERNEL:
                kprintf("trap_frame: handling with pause_kernel_trap_handler\n");
                status = pause_kernel_trap_handler(current_command, current_connection, ctx);
                break;
            default:
                kprintf("Unhandled command %d\n", current_command->header.command_type);  
        }

        if (yorha_trap_dbg_get_new_commands(command_data, 0x1000) == YORHA_FAILURE)
        {
            kprintf("Error reading new commands!\n");
            break;
        }   
        current_command = (dbg_command*) command_data;
    }
   

    return status;
}

//
// Read new commands using the debug loop thread and connection socket
//
int yorha_trap_dbg_get_new_commands(uint8_t* buff, size_t buff_size)
{
    kprintf("Starting trap frame command handler...\nReading data inside the gate...\n");

    if (kread(current_connection, buff, buff_size, main_thread) > 0)
    {
        kprintf("Read something...");
        return YORHA_SUCCESS;
    }
    
    kprintf("read failed\n");

    return YORHA_FAILURE;
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
            // check mtx
            kprintf("Reading data (outside gate)\n");
            if ((cmd_size = kread(conn, command_data, 0x1000, td)) > 0)
            {
                dbg_command* command = (dbg_command*) command_data;
                // mtx_lock -> should be unlocked in the trap fame handler
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
            case STOP_DBG:
                return stop_debugger_executor(command, conn);
            default:
                kprintf("Unhandled command %d\n", command->header.command_type);
        }
        // command_executor executor = command_executor_handlers[command->header.command_type];
        // return executor(command, conn);
    }

    kprintf("Invalid command received %d\n", command->header.command_type);

    return YORHA_FAILURE;
}

//
// Launch the int3 in a separated kthread
//
int pause_kernel_executor(dbg_command*, int)
{
    kproc_create(__debugbreak, 0, 0, 0, 0, "__debugbreak");
    return YORHA_SUCCESS;
}


int pause_kernel_trap_handler(dbg_command*, int, trap_frame_t* ctx)
{
    //
    // Reply the current register state and instruction pointer
    // 
    pause_kernel_response_data_t response = {0};

    memcpy(&response.trap_frame, &ctx, sizeof(trap_frame_t));
    //
    // TODO: Read X bytes from RIP, verify if the memory is safe to read
    //

    //
    // TODO: Verify if RIP + PAUSE_KERNEL_CODE_DUMP_SIZE is a valid kernel executable address!
    //
    memcpy(response.code, (const void*) ctx->rip, PAUSE_KERNEL_CODE_DUMP_SIZE);

    
    response.header.command_type = PAUSE_KERNEL;
    response.header.command_status = YORHA_SUCCESS;
    response.header.response_size = sizeof(response);

    int res = ksendto(current_connection, &response, sizeof(response), 0, 0, 0, main_thread);
    if (res < 0)
    {
        kprintf("Error calling ksendto!\n");
        return YORHA_FAILURE;
    }
    
    return YORHA_SUCCESS;
}


int stop_debugger_executor(dbg_command*, int)
{
    kprintf("Exiting debugger...\n");
    dbg_status = STOPPED;

    return YORHA_SUCCESS;
}


