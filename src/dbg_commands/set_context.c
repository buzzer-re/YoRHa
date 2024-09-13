#include "../../include/yorha_dbg_commands.h"


//
// This command only will be called inside a trap handler
//
int set_thread_context_executor(dbg_command_t*, int)
{
    return YORHA_FAILURE;
}


//
// Copy the request register values to the trapped thread one
//
int set_thread_context_trap_handler(dbg_command_t* command, int remote_connection, trap_frame_t* ctx)
{
    kprintf("set_thread_context_trap_handler\n");
    if (command->header.command_type != DBG_SET_THREAD_CONTEXT || command->header.argument_size != sizeof(thread_context_request_t))
    {
        kprintf("Wrong thread_context_request_t data!\n");
        return YORHA_FAILURE;
    }
    
    thread_context_request_t ctx_request = {0};

    if (kread(remote_connection, (void*) &ctx_request, sizeof(thread_context_request_t), curthread) != (int) command->header.argument_size)
    {
        kprintf("Wrong or incomplete set_context data!\n");
        return YORHA_FAILURE;
    }

    memcpy(ctx, &ctx_request.ctx, sizeof(trap_frame_t));

    return YORHA_SUCCESS;
}
