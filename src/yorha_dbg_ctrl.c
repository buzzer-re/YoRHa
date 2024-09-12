#include "../include/yorha_dbg_ctrl.h"

#define DBG_HANDLE_LOOP while (yorha_handle_command(conn)) continue

int sock = -1;
enum DbgStatus dbg_status = IDLE;
struct sockaddr_in sockaddr;
int current_connection;
struct thread* main_thread;
dbg_command_t* current_command = NULL;


int yorha_dbg_breakpoint_handler(trap_frame_t* ctx)
{
    if (!kprintf)
    {
        // Should never be called, but who knows
        init_kernel();
    }

    kprintf("Calling yorha_dbg_main_trap_handler ");
    return yorha_dbg_main_trap_handler(ctx, current_command);
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
    dbg_command_t command = {0};

    size_t cmd_size;
    int conn;
    struct thread* td = curthread;

    sock = listen_port(port, td, true);
    if (sock < 0)
    {
        kprintf("Unable to listen at port %d!\n", port);
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
            //
            // TODO: Send welcome packet with the kernel information
            //
        read_data:
            if ((cmd_size = kread(conn, (void*) &command, sizeof(dbg_command_t), td)) > 0)
            {                
                if (yorha_dbg_handle_command(&command, conn) != DBG_STOP)
                {
                    kprintf("Waiting next commands...\n");
                    goto read_data;
                }

                kprintf("An error happened when processing command %d from remote, closing connection...\n", command.header.command_type);
            }
            
            kclose(conn, td);
        } 
        kshutdown(sock, SHUT_RDWR, td);
        kclose(sock, td);
    }

    return YORHA_SUCCESS;
}


//
// Handle commands that are supported in a non-paused state
//
int yorha_dbg_handle_command(dbg_command_t* command, int conn)
{
    if (command->header.command_type <= __max_dbg_commands )
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
            case DBG_PAUSE:
                return pause_kernel_executor(command, conn);

            case DBG_STOP:
                return stop_debugger_executor(command, conn);

            case DBG_PLACE_BREAKPOINT:
                return place_breakpoint_executor(command, conn);

            case DBG_KPAYLOAD_LOADER:
                return kpayload_loader_executor(command, conn);
                
            case DBG_LIST_BREAKPOINT:
                return list_breakpoint_executor(command, conn);

            case DBG_REMOVE_BREAKPOINT:
                return remove_breakpoint_executor(command, conn);

            case DBG_MEM_READ:
                return memory_read_executor(command, conn);

            case DBG_MEM_WRITE:
                return memory_write_executor(command, conn);
                    

            default:
                kprintf("Invalid command received %d\n", command->header.command_type);
        }
    }

    return YORHA_FAILURE;
}

//
// Issue a int3 instruction on the current thread to give control to the trap handler
//
int pause_kernel_executor(dbg_command_t*, int)
{
    __debugbreak();
    return YORHA_SUCCESS;
}

//
// "Stop" the debug loop, which will cause the IDT to be restored and the Yorha dbg ends
//
int stop_debugger_executor(dbg_command_t*, int)
{
    kprintf("Exiting debugger...\n");
    dbg_status = STOPPED;

    return YORHA_SUCCESS;
}

