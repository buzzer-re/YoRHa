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

    //if (!current_command || current_command->header.command_type > __max_dbg_trap_handlers) return YORHA_FAILURE;

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
// Load a remote code into a separated kproc, with a int3 instruction at the the begin to assist debugging
//
int kpayload_loader_executor(dbg_command_t* command, int conn)
{

    kprintf("kpayload_loader_executor\n");
    if (!command->header.argument_size)
    {
        kprintf("Wrong kloader argument size!");
        return YORHA_FAILURE;
    }

    kprintf("Argument_size -> %d\n", command->header.argument_size);
    kpayload_loader_request_t kloader_request = {0};

    //
    // Read the rest of the request data
    //
    if (kread(conn, &kloader_request, sizeof(kpayload_loader_request_t), curthread) != 9)
    {
        kprintf("Wrong or incomplete kpayload data!\n");
        return YORHA_FAILURE;
    }

    kprintf("Stop_At_entry: %d\nkpayload_size: %d\n", kloader_request.stop_at_entry, kloader_request.payload_size);
    //
    // The kmem_alloc above take account that the kmem_alloc patches were already applied by some 
    // jailbreak, such as Mira or Goldhen. Maybe in future I should add the same kernel patches as part of the YoRHa loading
    
    size_t alloc_size = kloader_request.stop_at_entry ? kloader_request.payload_size + 1 : kloader_request.payload_size;

    kprintf("Received KPayload with %d bytes!\n", alloc_size);

    uint8_t* exec_code = (uint8_t*) kmem_alloc(kernel_vmmap, alloc_size);
    uint8_t* code = exec_code;

    if (!exec_code)
    {
        kprintf("Unable to allocate kpayload code, system is out-of-memory or size to high!\n");
        return YORHA_FAILURE;
    }

    kprintf("kpayload address: 0x%llx\n", exec_code);

    if (kloader_request.stop_at_entry)
    {
        code[0] = INT3; // stop at the kpayload entry
        code++;
    }

    //
    // Read the kpayload data
    //
    size_t total_read = 0;
    kprintf("Reading kpayload...\n");
    do
    {
        total_read += kread(conn, code + total_read, kloader_request.payload_size, curthread);
    } while (total_read != kloader_request.payload_size);
    
    kprintf("Read %d bytes!\n", total_read);
    //
    // Exec in a separted kproc which will break inside the debugger
    //
    kproc_create( (void (*)(void *)) exec_code, NULL, NULL, NULL, NULL, "YorhaKLoaderPayload");

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

