#include "../../include/yorha_dbg_commands.h"


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
    size_t read_size;
    kprintf("Reading kpayload...\n");
    do
    {
        read_size = kread(conn, code + total_read, kloader_request.payload_size, curthread);
        total_read += read_size;
    } while (total_read != kloader_request.payload_size && read_size);
    
    if (total_read != kloader_request.payload_size)
    {
        kprintf("Unable to read full payload data! aborting...\n");
        return YORHA_FAILURE;
    }

    kprintf("Read %d bytes!\n", total_read);
    //
    // Exec in a separted kproc which will break inside the debugger
    //
    kproc_create( (void (*)(void *)) exec_code, NULL, NULL, 0, 0, "YorhaKLoaderPayload");

    return YORHA_SUCCESS;
}