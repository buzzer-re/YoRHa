#include "../include/yorha_dbg.h"


//
// Init the YorHa debugget, get the IDTR address, overwrite the Gate descriptors related to debugging
// Prepare initial structures for debugging over network
//
int yorha_dbg_init()
{
    //
    // Get IDT address
    //
    printf_debug("Getting IDT address...");

    struct IDTR idtr;
    __sidt(&idtr);
    
    printf_notification("IDT Base Address: 0x%p\n", idtr.base);
    printf_debug("IDT Base Address: 0x%p\n", idtr.base);


    display_idt_gates((uint64_t*) &idtr.base);

    return YORHA_SUCCESS;
}

void display_idt_gates(uint64_t* idt_base)
{
    printf_notification("Listing...");
    uint8_t* idt_entry = (uint8_t*) *idt_base;

    for (int i = 0; i < 10; ++i)
    {   
        uint64_t* entry = (uint64_t*) idt_entry;
        printf_debug("Gate address %d -> %p\n", i, entry);
        idt_entry += 0x10 * i;
    }
    
}