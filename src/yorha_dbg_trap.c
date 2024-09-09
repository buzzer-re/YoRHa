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

                case DBG_CONTEXT:
                    status = pause_kernel_trap_handler(command, remote_connection, ctx);
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
    dbg_mem_read_request_t read_request;

    if (kread(remote_connection, (void*) &read_request, sizeof(dbg_mem_read_request_t), curthread) != command->header.argument_size)
    {
        kprintf("Wrong or incomplete mem_read command data!\n");
        return YORHA_FAILURE;
    }

    int fail = 0;
    size_t total_size = sizeof(dbg_mem_read_response_t) + read_request.read_size;
    //
    // Alloc response struct
    //
    kprintf("Allocating %d bytes to read address %llx\n", total_size, read_request.target_addr);
    dbg_mem_read_response_t* response = (dbg_mem_read_response_t*) kmalloc(total_size, KM_TEMP, M_WAITOK | M_ZERO); 

    if (!response)
    {
        kprintf("malloc() fail, system is out of memory!\n");
        return YORHA_FAILURE;
    }

    // int old_flags = disable_thread_pf();
    if (read_request.read_size > PS4_PAGE_SIZE)
    {
        // 
        // Paginate read
        //
        size_t chunks = read_request.read_size / PS4_PAGE_SIZE;
        size_t remain = read_request.read_size % PS4_PAGE_SIZE;
        kprintf("%d chunks, remaining %d bytes\n", chunks, remain);
        for (size_t i = 0; i < chunks && !fail; ++i)
        {
            kprintf("memread chunk %d (0x%llx\n)\n", i, read_request.target_addr + (PS4_PAGE_SIZE * i));
            fail = kcopyin(read_request.target_addr + (PS4_PAGE_SIZE * i), &response->data[PS4_PAGE_SIZE * i], PS4_PAGE_SIZE);
        }
        
        if (remain && !fail)
        {
            fail = kcopyin(read_request.target_addr + (PS4_PAGE_SIZE * chunks), &response->data[PS4_PAGE_SIZE * chunks], remain);
        }
    }
    else
    {
        fail = kcopyin(read_request.target_addr, response->data, read_request.read_size);
    }

    // update_thread_flags(old_flags);

    response->header.command_type = DBG_MEM_READ;
    //
    // Maybe add a switch case here to describe which error actually happened, accoding to the manual
    // https://man.freebsd.org/cgi/man.cgi?query=copyin&sektion=9
    // 
    if (!fail)
    {
        response->header.response_size = read_request.read_size;
        response->header.command_status = YORHA_SUCCESS;
    }
    else
    {
        kprintf("memread: Failed to read with code %d\n", fail);
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