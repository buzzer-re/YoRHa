#include "../include/yorha.h"

uint64_t old_int3_gate_addr;

//
// Init the YorHa debugger, get the IDTR address, overwrite the Gate descriptors related to debugging
// Prepare initial structures for debugging over network
//
int yorha_dbg_start(void*, void*)
{
    init_kernel();
    kprintf("Kernel Base address: %p\n", kernel_base);
    int status = YORHA_SUCCESS;

    kprintf("Setting up IDT debug handlers...\n");
    //
    // Apply custom IDT handlers
    //
    overwrite_idt_gate(3, (uint64_t) &int_breakpoint_handler);

    //
    // Start the debug loop, in theory we should "never" exit this loop, 
    // exiting the debug loop means that the debugger should be disabled
    //
    LOG("Starting network debugger on port %d...", DBG_CTRL_PORT);
    if (yorha_dbg_run_debug_server_loop(DBG_CTRL_PORT) != YORHA_SUCCESS)
    {
        kprintf("Unable to execute YoRHdbg debug server! aborting...\n");
        status = YORHA_FAILURE;
    }

    kprintf("YoRHa dbg exiting...");

    overwrite_idt_gate(3, (uint64_t) old_int3_gate_addr);
    return status;
}


void overwrite_idt_gate(int interruption_number, uint64_t gate_addr)
{
    LOG("Patching gate %d with 0x%llx\n", interruption_number, gate_addr);
    
    enable_safe_patch();

    idtr idtr = { 0 };
    idt_64* idt_array;
    idt_64* idt_entry;
    __sidt(&idtr);

    LOG("Found IDT at %llx\n", idtr.base);

    idt_array = (idt_64*) idtr.base;
    idt_entry = (idt_64*) &idt_array[interruption_number];
    old_int3_gate_addr = UNPACK_HANDLER_ADDR(idt_entry);
    
    LOG("Interruption %d (0x%llx) handler is at -> 0x%llx\n", interruption_number, idt_entry, UNPACK_HANDLER_ADDR(idt_entry));

    idt_entry->offset_low     = (gate_addr & 0xFFFF);
    idt_entry->offset_middle  = (gate_addr >> 16 ) & 0xFFFF;
    idt_entry->offset_high    = (gate_addr >> 32 ) & 0xFFFFFFFF;

    LOG("Interruption %d (0x%llx) handler now is at -> 0x%llx\n", interruption_number, idt_entry, UNPACK_HANDLER_ADDR(idt_entry));

    disable_safe_patch();

}










