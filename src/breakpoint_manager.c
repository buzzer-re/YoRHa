#include "../include/breakpoint_manager.h"


SLIST_HEAD(slisthead, __breakpoint_entry) head = SLIST_HEAD_INITIALIZER(head);
struct slisthead* headp;

int initialized = false;

int add_breakpoint(uint64_t* addr)
{

    if (!initialized)
    {
        SLIST_INIT(&head);
        initialized = true;
    }

    kprintf("Adding breakpoint at %llx\n", addr);
    
    uint8_t old_opcode;
    uint8_t breakpoint_opcode = INT3;

    int flags = disable_thread_pf();
    int r = kcopyin(addr, &old_opcode, 1);
    update_thread_flags(flags);

    if (r)
    {
        //
        // Failed to read data
        //
        kprintf("Error reading old_opcode\n");
        return false;
    }
    
    kprintf("Old opcode: %x\n", old_opcode);

//    stop_other_cpus();
    enable_safe_patch();
    flags = disable_thread_pf();
    r = kcopyin(&breakpoint_opcode, addr, 1);
    update_thread_flags(flags);
    disable_safe_patch();

    if (r)
    {
        kprintf("Error applying INT3 patch!\n");
        return false;
    }  
  //  restart_cpus();

    kprintf("Done\nSaving entry on the kernel linked list...\n");

    breakpoint_entry_t* new_entry = (breakpoint_entry_t*) kmalloc(sizeof(breakpoint_entry_t), KM_TEMP, M_ZERO | M_WAITOK);

    new_entry->address = addr;
    new_entry->old_opcode = old_opcode;
    new_entry->enabled = true;

    SLIST_INSERT_HEAD(&head, new_entry, entries);
    kprintf("Done\n");

    return true;
}

