#include "../../include/yorha_dbg_commands.h"
//
// Place breakpoint executor, given an address verify if is in kernel space range, and replace the byte with int3
// If the page is not executable, an error will be sent to the user
//
int place_breakpoint_executor(dbg_command_t* command, int conn)
{
    if (command->header.argument_size != sizeof(breakpoint_request_t))
    {
        kprintf("Wrong argument size! got %d expected %d\n", command->header.argument_size, sizeof(breakpoint_request_t));
        return YORHA_FAILURE;
    }

    //
    // Read breakpoint request
    //
    breakpoint_request_t breakpoint_request = {0};

    if (kread(conn, (void*) &breakpoint_request, sizeof(breakpoint_request_t), curthread) != (int) command->header.argument_size)
    {
        kprintf("Wrong or incomplete breakpoint command data!\n");
        return YORHA_FAILURE;
    }

    if (!add_breakpoint(breakpoint_request.target_address))
    {
        kprintf("Error setting breakpoint!\n");
    }

    return YORHA_SUCCESS;
}


//
// List all current registered breakpoints
//
int list_breakpoint_executor(dbg_command_t*, int conn)
{
    // return list_breakpoints_shared_cmd(NULL, conn);
    kprintf("list_breakpoint_executor\n");
    size_t num_breakpoints = 0;
    int status = YORHA_SUCCESS;
    size_t total_breakpoints_bytes = 0;
    size_t alloc_size;
    breakpoint_entry_t* breakpoint_list = get_breakpoint_addresses(&num_breakpoints);
    breakpoint_list_response_t* response;
    
    if (!breakpoint_list || !num_breakpoints)
    {
        kprintf("Error on get_breakpoint_addresses\n");
        alloc_size = sizeof(breakpoint_list_response_t);
        response = kmalloc(alloc_size, KM_TEMP, M_WAITOK | M_ZERO);
        status = YORHA_FAILURE;
        goto send;
    } 

    total_breakpoints_bytes = sizeof(breakpoint_entry_t) * num_breakpoints;
    alloc_size = num_breakpoints * sizeof(breakpoint_entry_t) + sizeof(breakpoint_list_response_t);
    response = kmalloc(alloc_size, KM_TEMP, M_WAITOK | M_ZERO);

    if (!response)
    {
        kprintf("System is out-of-memory!\n");
        return YORHA_FAILURE;
    }

    memcpy(&response->breakpoint_entry, breakpoint_list, total_breakpoints_bytes);
    
send:    
    response->header.command_status = status;
    response->header.command_type = DBG_LIST_BREAKPOINT;
    response->header.response_size = total_breakpoints_bytes;
    response->num_breakpoints = num_breakpoints;

    int res = ksendto(conn, response, alloc_size, 0, 0, 0, curthread);

    kfree(response, KM_TEMP);

    if (res < 0)
    {
        kprintf("list_breakpoint_executor(): Error calling ksendto!\n");
        return YORHA_FAILURE;
    }
    
    
    return YORHA_SUCCESS;
}


//
// Remove the breakpoint from the linked list and patch back the code with the original opcode
//
int remove_breakpoint_executor(dbg_command_t* command, int conn)
{
    if (command->header.argument_size != sizeof(breakpoint_remove_request_t))
    {
        kprintf("remove_breakpoint_executor: Wrong argument size! got %d bytes expected %d\n", command->header.argument_size, sizeof(breakpoint_remove_request_t));
        return YORHA_FAILURE;
    }

    breakpoint_remove_request_t request = {0};

    if (kread(conn, (void*) &request, sizeof(breakpoint_remove_request_t), curthread) != (int) command->header.argument_size)
    {
        kprintf("remove_breakpoint_executor: Wrong or incomplete breakpoint command data!\n");
        return YORHA_FAILURE;
    }

    if (!remove_breakpoint(request.target_addr))
    {
        kprintf("Unable to remove breakpoint %llx\n", request.target_addr);
        return YORHA_FAILURE;
    }

    return YORHA_SUCCESS;
}
