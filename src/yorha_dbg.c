#include "../include/yorha_dbg.h"


void yorha_breakpoint_gate_handler()
{
    kprintf("breakpoint called");
}

//
// Init the YorHa debugger, get the IDTR address, overwrite the Gate descriptors related to debugging
// Prepare initial structures for debugging over network
//
int yorha_dbg_init(void*, void*)
{
    init_kernel();
    
    //
    // Apply custom IDT handlers
    //
    overwrite_idt_gate(3, &yorha_breakpoint_gate_handler);

    __asm__("int3");
    return YORHA_SUCCESS;
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

    LOG("Interruption %d (0x%llx) handler is at -> 0x%llx\n", interruption_number, idt_entry, UNPACK_HANDLER_ADDR(idt_entry));

    idt_entry->offset_low     = (gate_addr & 0xFFFF);
    idt_entry->offset_middle  = (gate_addr >> 16 ) & 0xFFFF;
    idt_entry->offset_high    = (gate_addr >> 32 ) & 0xFFFFFFFF;

    LOG("Interruption %d (0x%llx) handler is at -> 0x%llx\n", interruption_number, idt_entry, UNPACK_HANDLER_ADDR(idt_entry));

    disable_safe_patch();
}










