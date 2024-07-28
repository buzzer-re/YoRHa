#include "../include/yorha_dbg_trap.h"


int remote_connection;
dbg_command* command;

int yorha_dbg_main_trap_handler(trap_frame_t* ctx, dbg_command* cmd)
{
     // command_trap_handler trap_handler;
    int status = YORHA_SUCCESS;
    command = cmd;
    //
    // Stop all others CPU's to completaly freeze the system
    //
    kprintf("YorhaDBG Trap handler called, freezing the system...");


    status = yorha_trap_command_handler(ctx);
    //restart_cpus();

    kprintf("Resuming execution...\n");
    
    return status;
}

int yorha_trap_command_handler(trap_frame_t* ctx)
{
    //
    // Trap frame command handler
    //
    uint8_t command_data[0x1000] = {0};
    int status = YORHA_SUCCESS;
    int cmd_loop = true;
    int sock = listen_port(DBG_TRAP_PORT);

    if (sock < 0)
    {
        kprintf("Unable to listen port %d, aborting command_loop\n", DBG_TRAP_PORT);
        return YORHA_FAILURE;
    }

    kprintf("Wait commands at the trap handler");
   // stop_other_cpus();
    do
    {    
        remote_connection = kaccept(sock, NULL, NULL, curthread);

        if (remote_connection < 0)
        {
            kprintf("Error handling connection...aborting\n");
            break;
        }
        
        kprintf("Processing command %d\n", command->header.command_type);

        while (true)
        {
            switch (command->header.command_type)
            {
                case DBG_PAUSE:
                    kprintf("trap_frame: handling with pause_kernel_trap_handler\n");
                    status = pause_kernel_trap_handler(command, remote_connection, ctx);
                    break;
                case DBG_PLACE_BREAKPOINT:
                    kprintf("Trap frame: handling with place_breakpoint_trap_handler");
                    status = place_breakpoint_trap_handler(command, remote_connection, ctx);
                    break;
                case DBG_CONTINUE:
                    cmd_loop = false;
                    goto close;
                default:
                    kprintf("Unhandled command %d\n", command->header.command_type);  
            }

            if (yorha_trap_dbg_get_new_commands(command_data, 0x1000, remote_connection) == YORHA_FAILURE)
            {
                kprintf("Error reading new commands!\n");
                cmd_loop = false;
                goto close;
                break;
            }
            
            command = (dbg_command*) command_data;
        }

    close:
        kclose(remote_connection, curthread);

    } while (cmd_loop);
   
    return status;
}


//
// Read new commands using the debug loop thread and connection socket
//
int yorha_trap_dbg_get_new_commands(uint8_t* buff, size_t buff_size, int conn)
{
    kprintf("Starting trap frame command handler...\nReading data inside the gate...\n");

    if (kread(conn, buff, buff_size, curthread) > 0)
    {
        kprintf("Read something...");
        return YORHA_SUCCESS;
    }
    
    kprintf("read failed\n");

    return YORHA_FAILURE;
}

//
// Acquire the information that the kernel had before the pause and send back to the user
//
int pause_kernel_trap_handler(dbg_command*, int, trap_frame_t* ctx)
{
    pause_kernel_response_data_t response = {0};
    memcpy(&response.trap_frame, &ctx, sizeof(trap_frame_t));
    //
    // TODO: Read X bytes from RIP, verify if the memory is safe to read
    //

    //
    // TODO: Verify if RIP + PAUSE_KERNEL_CODE_DUMP_SIZE is a valid kernel executable address!
    //
    memcpy(response.code, (const void*) ctx->rip, PAUSE_KERNEL_CODE_DUMP_SIZE);

    
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
int place_breakpoint_trap_handler(dbg_command* command, int conn, trap_frame_t* ctx)
{
    return pause_kernel_trap_handler(command, conn, ctx);
}
