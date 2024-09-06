#include "../include/yorha_dbg_trap.h"


int remote_connection;
dbg_command_t* command;

int yorha_dbg_main_trap_handler(trap_frame_t* ctx, dbg_command_t* cmd)
{
    command = cmd;
    //
    // Stop all others CPU's to completaly freeze the system
    //
  
    // uint64_t intr = intr_disable();
    kprintf("dbg_trap_handler called!\n");
    __asm__("sti");
    int status = yorha_trap_command_handler(ctx);
    // intr_restore(intr);

    if (cmd->header.command_type != DBG_PAUSE)
    {
        //remove_breakpoint(--ctx->rip);
    }

    return status;
}


int yorha_trap_command_handler(trap_frame_t* ctx)
{
    //
    // Trap frame command handler
    //
    uint8_t command_data[0x1000] = {0}; // TODO: remove this from stack

    //
    // The trap handler was called outside the dbg controller context
    //
    if (!command)
        command = command_data;

    int status = YORHA_SUCCESS;
    int cmd_loop = true;
    int remote_fd_flags;
    struct thread* td = curthread;
    int sock = listen_port(DBG_TRAP_PORT, td, 1);

    if (sock < 0)
    {
        kprintf("Unable to listen port %d, aborting command_loop\n", DBG_TRAP_PORT);
        return YORHA_FAILURE;
    }

    kprintf("Wait commands at the trap handler\n");
    
    do
    {    
        kprintf("Waiting new connections...\n");
        remote_connection = kaccept(sock, NULL, NULL, td);

        if (remote_connection < 0)
        {
            kprintf("Error handling connection...aborting\n");
            break;
        }
        
        //
        // Make the fd non-block to not lock the system
        //
        remote_fd_flags = kfcntl(remote_connection, F_GETFL, NULL, td);
        kfcntl(remote_connection, F_SETFL, remote_fd_flags | O_NONBLOCK, td);

        kprintf("Processing command %d\n", command->header.command_type);
        
      //  STOP();
        while (true)
        {
            switch (command->header.command_type)
            {

                case DBG_PAUSE:
                    kprintf("trap_frame: DBG is paused\n");
                    break;
                
                case DBG_PLACE_BREAKPOINT:
                    kprintf("Trap frame: handling with place_breakpoint_trap_handler");
                    //status = place_breakpoint_trap_handler(command, remote_connection, ctx);
                    break;
                
                case DBG_CONTINUE:
                    kprintf("Exiting trap frame...\n");
                    cmd_loop = false;
                    goto close;

                case DBG_MEM_READ:
                    status = memory_read_trap_handler(command, remote_connection, ctx);
                    break;

                case DBG_CONTEXT:
                    status = pause_kernel_trap_handler(command, remote_connection, ctx);
                    break;

                default:
                    kprintf("Unhandled command %d\n", command->header.command_type);  
            }

            if (yorha_trap_dbg_get_new_commands(command_data, 0x1000, remote_connection, td) == YORHA_FAILURE)
            {
                kprintf("Error reading new commands!\n");
                //cmd_loop = false;
                goto close;
                break;
            }
            
            command = (dbg_command_t*) command_data;
        }

    close:
    //    RESTART();
        kclose(remote_connection, td);

    } while (cmd_loop);
    
    kprintf("Closing socket on trap frame handler...\n");
  //  RESTART(); // contains a internal check
  
    int r = kshutdown(sock, SHUT_RDWR, td);
    
    if (r == -4)
    {
        kprintf("Interrupted syscall!\n");
    }

    kclose(sock, td);

    return status;
}


//
// Read new commands using the debug loop thread and connection socket
//
int yorha_trap_dbg_get_new_commands(uint8_t* buff, size_t buff_size, int conn, struct thread* td)
{
    fd_set readfds;
    int status;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(conn, &readfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    kprintf("Starting trap frame command handler...\nReading data inside the gate...\n");
    
    while (1)
    {
        status = kselect(conn + 1, &readfds, NULL, NULL, NULL, td);
        kprintf("Called select()\n");
        
        if (status < 0)
        {
            // kprintf("select() error when reading remote, aborting...\n");
            continue;
        }

        if (FD_ISSET(conn, &readfds))
        {
            // ugly and dirty hack to make read work, it will not "block" since the fd is ready from the "select()" syscall
       //     RESTART(); 
            int length = kread(conn, buff, buff_size, td);
         //   STOP();

            if (!length)
            {
                //
                // Socket is not ready anymore, read only returns 0
                //
                kprintf("Connection not valid anymore!\n");
                break;
            }
            return YORHA_SUCCESS;
        }
    }
    
    return YORHA_FAILURE;
}

//
// Acquire the information that the kernel had before the pause and send back to the user
//
int pause_kernel_trap_handler(dbg_command_t*, int, trap_frame_t* ctx)
{
    kprintf("pause_kernel_trap_handler\n");
    pause_kernel_response_data_t response = {0};
    memcpy(&response.trap_frame, ctx, sizeof(trap_frame_t));
    //
    // TODO: Read X bytes from RIP, verify if the memory is safe to read
    //

    //
    // TODO: Verify if RIP + PAUSE_KERNEL_CODE_DUMP_SIZE is a valid kernel executable address!
    //
    memcpy(response.code, (const void*) ctx->rip - 1, PAUSE_KERNEL_CODE_DUMP_SIZE);
    response.header.command_type = DBG_PAUSE;
    response.header.command_status = YORHA_SUCCESS;
    response.header.response_size = sizeof(response);
    
    int res = ksendto(remote_connection, &response, sizeof(response), 0, 0, 0, curthread);
    if (res < 0)
    {
        kprintf("Error calling ksendto!\n");
        return YORHA_FAILURE;
    }
    
    return YORHA_SUCCESS;
}

//
// For the response, we can use the same structure as the pause
//
int place_breakpoint_trap_handler(dbg_command_t* command, int conn, trap_frame_t* ctx)
{
    return pause_kernel_trap_handler(command, conn, ctx);
}
//
// Read memory data
//
int memory_read_trap_handler(dbg_command_t* request, int remote_connection, trap_frame_t*)
{
    kprintf("memory_read_trap_handler called\n");

    if (request->header.command_type != DBG_MEM_READ || request->header.argument_size != sizeof(dbg_mem_read_request_t))
    {
        kprintf("Invalid request\nargument_size %d expected %d\n", request->header.argument_size, sizeof(dbg_mem_read_request_t));
        return YORHA_FAILURE;
    }

    int status = YORHA_SUCCESS;
    dbg_mem_read_request_t* read_request = NULL;
    size_t total_size = sizeof(dbg_mem_read_response_t) + read_request->read_size;
    //
    // Alloc response struct
    //
    dbg_mem_read_response_t* response = (dbg_mem_read_response_t*) kmalloc(total_size, KM_TEMP, M_WAITOK | M_ZERO); 

    if (!response)
    {
        kprintf("malloc() fail, system is out of memory!\n");
        return YORHA_FAILURE;
    }

    int old_flags = disable_thread_pf();
    int fail = kcopyin(read_request->target_addr, response->data, read_request->read_size);
    update_thread_flags(old_flags);

    response->header.command_type = DBG_MEM_READ;
    //
    // Maybe add a switch case here to describe which error actually happened, accoding to the manual
    // https://man.freebsd.org/cgi/man.cgi?query=copyin&sektion=9
    // 
    if (!fail)
    {
        response->header.response_size = read_request->read_size;
        response->header.command_status = YORHA_SUCCESS;
    }
    else
    {
        response->header.command_status = YORHA_INVALID_MEM_ADDRESS;
        response->header.response_size = 0;
    }

    int res = ksendto(remote_connection, response, total_size, 0, 0, 0, curthread);
    if (res < 0)
    {
        kprintf("Error calling ksendto!\n");
        status = YORHA_FAILURE;
    }

    kfree(response, KM_TEMP);

    return status;
}