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

#ifdef _DEBUG
    display_idt_gates();
#endif

    return YORHA_SUCCESS;
}

void display_idt_gates()
{

}