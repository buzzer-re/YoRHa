#include "../../include/yorha_dbg_commands.h"


int single_step_trap_handler(dbg_command_t*, int, trap_frame_t* ctx)
{
    //
    // Enable Trap Flag (TF)
    //

    ctx->eflags |= TF;
    return YORHA_SUCCESS;
}
