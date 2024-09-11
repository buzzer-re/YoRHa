#include "../include/yorha_dbg_trap.h"


int remote_connection;
dbg_command_t* command;

int yorha_dbg_main_trap_handler(trap_frame_t* ctx, dbg_command_t* cmd)
{
    command = cmd;
    __asm__("sti");
    int status = yorha_trap_command_handler(ctx);

    //
    // Restore old opcode from the soft breakpoint
    //
    if (cmd->header.command_type != DBG_PAUSE && get_breakpoint_entry(ctx->rip - 1))
    {
        remove_breakpoint(--ctx->rip);
        //
        // otherwise, this is not a user controlled intr
        //
    }

    return status;
}


int yorha_trap_command_handler(trap_frame_t* ctx)
{
    //
    // Trap frame command handler
    //
    // TODO: remove this from stack
    dbg_command_t current_command = {0};

    //
    // The trap handler was called outside the dbg controller context
    //
    if (!command)
        command = &current_command;

    int status = YORHA_SUCCESS;
    int cmd_loop = true;
    int remote_fd_flags;
    struct thread* td = curthread;
    int sock = listen_port(DBG_TRAP_PORT, td, true);

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
                
                case DBG_CONTINUE:
                    kprintf("Exiting trap frame...\n");
                    cmd_loop = false;
                    goto close;

                case DBG_MEM_READ:
                    status = memory_read_trap_handler(command, remote_connection, ctx);
                    break;

                case DBG_MEM_WRITE:
                    status = memory_write_trap_handler(command, remote_connection, ctx);
                    break;

                case DBG_CONTEXT:
                    status = pause_kernel_trap_handler(command, remote_connection, ctx);
                    break;

                case DBG_SET_THREAD_CONTEXT:
                    status = set_thread_context_trap_handler(command, remote_connection, ctx);
                    break;
            
                case DBG_PLACE_BREAKPOINT:
                    status = place_breakpoint_executor(command, remote_connection);
                    break;

                case DBG_LIST_BREAKPOINT:
                    status = list_breakpoint_executor(command, remote_connection);
                    break;

                case DBG_REMOVE_BREAKPOINT:
                    status = remove_breakpoint_executor(command, remote_connection);
                    break;

                default:
                    kprintf("Unhandled command %d\n", command->header.command_type);  
            }

            if (yorha_trap_dbg_get_new_commands(command, remote_connection, td) == YORHA_FAILURE)
            {
                kprintf("Error reading new commands!\n");
                //cmd_loop = false;
                goto close;
                break;
            }            
        }

    close:
    //    RESTART();
        kclose(remote_connection, td);

    } while (cmd_loop);
    
    kprintf("Closing socket on trap frame handler...\n");
  //  RESTART(); // contains a internal check
  
    kshutdown(sock, SHUT_RDWR, td);
    kclose(sock, td);

    return status;
}


//
// Read new commands using the debug loop thread and connection socket
//
int yorha_trap_dbg_get_new_commands(dbg_command_t* command, int conn, struct thread* td)
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
            int length = kread(conn, command, sizeof(dbg_command_t), td);
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
    uint64_t rip = ctx->rip;
    memcpy(&response.trap_frame, ctx, sizeof(trap_frame_t));
    //
    // TODO: Read X bytes from RIP, verify if the memory is safe to read
    //

    //
    // TODO: Verify if RIP + PAUSE_KERNEL_CODE_DUMP_SIZE is a valid kernel executable address!
    //
   // memcpy(response.code, (const void*) rip - 1, PAUSE_KERNEL_CODE_DUMP_SIZE);
    
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
