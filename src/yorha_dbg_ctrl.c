#include "../include/yorha_dbg_ctrl.h"

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
    
    uint8_t command_data[0x1000];
    size_t cmd_size;
    int conn;
    struct thread* td = curthread;

    sock = listen_port(port, td);
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
            if ((cmd_size = kread(conn, command_data, 0x1000, td)) > 0)
            {
                dbg_command* command = (dbg_command*) command_data;
                if (yorha_dbg_handle_command(command, conn) != DBG_STOP)
                {
                    kprintf("Waiting next commands...\n");
                    goto read_data;
                }

                kprintf("Received DBG_STOP command from remote, closing connection...\n");
            }
            
            kclose(conn, td);
        } 

        kclose(sock, td);
    }

    return YORHA_SUCCESS;
}


//
// Handle commands that are supported in a non-paused state
//
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
            case DBG_PAUSE:
                return pause_kernel_executor(command, conn);
            case DBG_STOP:
                return stop_debugger_executor(command, conn);
            case DBG_PLACE_BREAKPOINT:
                return place_breakpoint_executor(command, conn);
            default:
                kprintf("Invalid command received %d\n", command->header.command_type);
        }
    }

    return YORHA_FAILURE;
}

//
// Issue a int3 instruction on the current thread to give control to the trap handler
//
int pause_kernel_executor(dbg_command*, int)
{
    __debugbreak();
    return YORHA_SUCCESS;
}

//
// Place breakpoint executor, given an address verify if is in kernel space range, and replace the byte with int3
// If the page is not executable, an error will be sent to the user
//
int place_breakpoint_executor(dbg_command* command, int)
{
    //
    // For safety, the breakpoint should be placed on a freezed state
    //
    // return pause_kernel_executor();

    if (command->header.argument_size != sizeof(breakpoint_request_t))
    {
        // return error
        kprintf("Wrong argument size! got %d expected %d\n", command->header.argument_size, sizeof(breakpoint_request_t));
        return YORHA_FAILURE;
    }
    uint8_t old_opcode;
    uint8_t* code_at;
    breakpoint_request_t* breakpoint_request = (breakpoint_request_t*) command->data;

    enable_safe_patch();

    kprintf("Placing breakpoint at %llx\n", breakpoint_request->target_address);
    code_at = (uint8_t*) *breakpoint_request->target_address;
    // where do I store that shit ?
    old_opcode = code_at[0];
    code_at[0] = INT3;
    
    kprintf("Old byte %x\n", old_opcode);

    disable_safe_patch();

    return YORHA_SUCCESS;
}


//
// "Stop" the debug loop, which will cause the IDT to be restored and the Yorha dbg ends
//
int stop_debugger_executor(dbg_command*, int)
{
    kprintf("Exiting debugger...\n");
    dbg_status = STOPPED;

    return YORHA_SUCCESS;
}


