#include "../include/breakpoint_manager.h"


SLIST_HEAD(slisthead, __breakpoint_entry) head = SLIST_HEAD_INITIALIZER(head);
struct slisthead* headp;
int initialized = false;
size_t num_breakpoints = 0;

int add_breakpoint(uint64_t* addr)
{

    if (!initialized)
    {
        SLIST_INIT(&head);
        initialized = true;
    }

    if (get_breakpoint_entry(addr) != NULL)
    {
         kprintf("Breakpoint already exists at %llx\n", addr);
        return true;
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

    breakpoint_entry_t* new_entry = (breakpoint_entry_t*) kmalloc(sizeof(breakpoint_entry_t), KM_TEMP, M_ZERO | M_WAITOK);

    new_entry->address = addr;
    new_entry->old_opcode = old_opcode;
    new_entry->enabled = true;

    SLIST_INSERT_HEAD(&head, new_entry, entries);
    num_breakpoints++;

    return true;
}

//
// Remove a breakpoint from a given address
//
int remove_breakpoint(uint64_t* addr)
{
    if (!num_breakpoints) return false;
    kprintf("Removing breakpoint %llx\n", addr);
    breakpoint_entry_t* np;

    SLIST_FOREACH(np, &head, entries)
    {
        if (np->address == addr)
        {
            enable_safe_patch();
            int flags = disable_thread_pf();
            kcopyin(&np->old_opcode, addr, 1);
            update_thread_flags(flags);
            disable_safe_patch();
            //
            // Remove from linked list
            //
            SLIST_REMOVE(&head, np, __breakpoint_entry, entries);
            num_breakpoints--;
            
            return true;
        }
    }

    return false;
}

//
// Get a single breakpoint entry information
//
breakpoint_entry_t* get_breakpoint_entry(uint64_t* addr)
{
    if (!num_breakpoints) return false;

    breakpoint_entry_t* np;
    SLIST_FOREACH(np, &head, entries)
    {
        if (np->address == addr)
        {
            return np;
        }
    }

    return NULL;
}

//
// Get all the current breakpoints
//
breakpoint_entry_t* get_breakpoint_addresses(size_t* len)
{
    if (!num_breakpoints)
        return NULL;

    breakpoint_entry_t* breakpoint_list =  (breakpoint_entry_t*) kmalloc(sizeof(breakpoint_entry_t) * num_breakpoints, KM_TEMP, M_ZERO | M_WAITOK);

    if (!breakpoint_list)
    {
        kprintf("get_breakpoint_addresses: kmalloc failed! System is out-of-memory!\n");
        return NULL;
    }
    
    breakpoint_entry_t* np;
    int i = 0;

    SLIST_FOREACH(np, &head, entries)
    {
        breakpoint_list[i].address = np->address;
        breakpoint_list[i].enabled = np->enabled;
        breakpoint_list[i].old_opcode = np->old_opcode;
        i++;
    }

    *len = num_breakpoints;
    
    return breakpoint_list;
}

