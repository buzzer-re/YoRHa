#include "../../include/yorha_dbg_commands.h"


int memory_read_executor(dbg_command_t* command, int conn)
{
    //
    // Wrapper
    //
    return memory_read_trap_handler(command, conn, NULL);
}

int memory_write_executor(dbg_command_t* command, int conn)
{
    return memory_write_trap_handler(command, conn, NULL);
}

//
// Read memory data
//
int memory_read_trap_handler(dbg_command_t* request, int remote_connection, trap_frame_t*)
{
    kprintf("memory_read_trap_handler called\n");

    if (request->header.command_type != DBG_MEM_READ || request->header.argument_size != sizeof(dbg_mem_read_request_t))
    {
        kprintf("Invalid request\nargument_size %d expected %d\n", request->header.argument_size, sizeof(dbg_mem_read_request_t));
        return YORHA_FAILURE;
    }

    int status = YORHA_SUCCESS;
    dbg_mem_read_request_t read_request;

    if (kread(remote_connection, (void*) &read_request, sizeof(dbg_mem_read_request_t), curthread) != (int) request->header.argument_size)
    {
        kprintf("Wrong or incomplete mem_read command data!\n");
        return YORHA_FAILURE;
    }

    int fail = 0;
    size_t total_size = sizeof(dbg_mem_read_response_t) + read_request.read_size;
    //
    // Alloc response struct
    //
    // kprintf("Allocating %d bytes to read address %llx\n", total_size, read_request.target_addr);
    dbg_mem_read_response_t* response = (dbg_mem_read_response_t*) kmalloc(total_size, KM_TEMP, M_WAITOK | M_ZERO); 

    if (!response)
    {
        kprintf("malloc() fail, system is out of memory!\n");
        return YORHA_FAILURE;
    }

    // int old_flags = disable_thread_pf();
    if (read_request.read_size > PS4_PAGE_SIZE)
    {
        // 
        // Paginate read
        //
        size_t chunks = read_request.read_size / PS4_PAGE_SIZE;
        size_t remain = read_request.read_size % PS4_PAGE_SIZE;
        // kprintf("%d chunks, remaining %d bytes\n", chunks, remain);
        for (size_t i = 0; i < chunks && !fail; ++i)
        {
            // kprintf("memread chunk %d (0x%llx\n)\n", i, read_request.target_addr + (PS4_PAGE_SIZE * i));
            fail = kcopyin(&read_request.target_addr[PS4_PAGE_SIZE * i], &response->data[PS4_PAGE_SIZE * i], PS4_PAGE_SIZE);
        }
        
        if (remain && !fail)
        {
            fail = kcopyin(&read_request.target_addr[PS4_PAGE_SIZE * chunks], &response->data[PS4_PAGE_SIZE * chunks], remain);
        }
    }
    else
    {
        fail = kcopyin(read_request.target_addr, response->data, read_request.read_size);
    }

    // update_thread_flags(old_flags);

    response->header.command_type = DBG_MEM_READ;
    //
    // Maybe add a switch case here to describe which error actually happened, accoding to the manual
    // https://man.freebsd.org/cgi/man.cgi?query=copyin&sektion=9
    // 
    if (!fail)
    {
        response->header.response_size = read_request.read_size;
        response->header.command_status = YORHA_SUCCESS;
    }
    else
    {
        kprintf("memread: Failed to read with code %d\n", fail);
        response->header.command_status = YORHA_INVALID_MEM_ADDRESS;
        response->header.response_size = 0;
    }

    int res = ksendto(remote_connection, response, total_size, 0, 0, 0, curthread);
    if (res < 0)
    {
        kprintf("Error calling ksendto!\n");
        status = YORHA_FAILURE;
    }

    kfree(response, KM_TEMP);

    return status;
}



//
// Read the mem_write data and write into the target address
//
int memory_write_trap_handler(dbg_command_t* request, int remote_connection, trap_frame_t*)
{
    kprintf("memory_write_trap_handler called\n");

    if (request->header.command_type != DBG_MEM_WRITE || request->header.argument_size != sizeof(dbg_mem_write_request_t))
    {
        kprintf("Invalid request\nargument_size %d expected %d\n", request->header.argument_size, sizeof(dbg_mem_write_request_t));
        return YORHA_FAILURE;
    }

    dbg_mem_write_request_t write_request = {0};

    if (kread(remote_connection, (void*) &write_request, sizeof(dbg_mem_write_request_t), curthread) != sizeof(dbg_mem_write_request_t))
    {
        kprintf("Wrong or incomplete mem_write command data!\n");
        return YORHA_FAILURE;
    }

    uint8_t* write_data = kmalloc(write_request.write_size, KM_TEMP, M_WAITOK | M_ZERO);

    if (!write_data)
    {
        kprintf("malloc() fail, system is out-of-memory!\n");
        return YORHA_FAILURE;
    }

    size_t total_read = 0;
    size_t read_size;
    int fail = 0;

    kprintf("Reading mem_Write data...\n");
    do
    {
        read_size = kread(remote_connection, &write_data[total_read], write_request.write_size, curthread);
        total_read += read_size;
    } while (total_read != write_request.write_size && read_size);

    kprintf("Read %d bytes from %d\n", total_read, write_request.write_size);
    
    if (total_read != write_request.write_size)
    {
        kprintf("Unable to read full write_request data! aborting...\n");
        return YORHA_FAILURE;
    }

    kprintf("Writing memory...\n");
    enable_safe_patch();

    if (write_request.write_size > PS4_PAGE_SIZE)
    {
        // 
        // Paginate write
        //
        size_t chunks = write_request.write_size / PS4_PAGE_SIZE;
        size_t remain = write_request.write_size % PS4_PAGE_SIZE;
        kprintf("%d chunks, remaining %d bytes\n", chunks, remain);
        for (size_t i = 0; i < chunks && !fail; ++i)
        {
            kprintf("memwrite chunk %d (0x%llx\n)\n", i, &write_request.target_addr[PS4_PAGE_SIZE * i]);
            fail = kcopyin(&write_data[PS4_PAGE_SIZE * i], &write_request.target_addr[PS4_PAGE_SIZE * i], PS4_PAGE_SIZE);
        }
        
        if (remain && !fail)
        {
            fail = kcopyin(&write_data[PS4_PAGE_SIZE * chunks], &write_request.target_addr[PS4_PAGE_SIZE * chunks], remain);
        }
    }
    else
    {
        fail = kcopyin(write_data, write_request.target_addr, write_request.write_size);
    }

    kprintf("Data written!\n");
    
    disable_safe_patch();
    kfree(write_data, KM_TEMP);


    return YORHA_SUCCESS;
}