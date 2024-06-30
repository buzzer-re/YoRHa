#include "../include/kernel.h"

void (*critical_enter)();
void (*critical_exit)();
int (*kprintf)(const char *format, ...) = (int(*)(const char *, ...)) NULL;
int (*kprintf)(const char *format, ...);
int (*ksock_create)(void **socket, int domain, int type, int protocol);
int (*ksock_close)(void *socket);
int (*ksock_bind)(void *socket, struct sockaddr *addr);
int (*ksock_recv)(void *socket, void *buf, size_t *len);
int (*kproc_create)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...);

int (*ksys_socket)(struct thread* td, struct socket_args* uap);
int (*ksys_bind)(struct thread* td, struct bind_args* uap);
int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
int (*ksys_close)(struct thread* td, struct close_args* uap);
int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
int (*ksys_read)(struct thread* td, struct read_args* uap);
int (*ksys_accept)(struct thread* td, struct accept_args* uap);
int (*ksys_listen)(struct thread* td, struct listen_args* uap);
uint8_t* (*kmem_alloc)(vm_map_t map, size_t size);
void (*kmem_free)(vm_map_t map, void* addr, size_t size);

uint8_t* kernel_base = 0;
struct sysent* sysents;

//
// Kernel memory structs
//

vm_map_t kernel_vmmap;

//
// Init kernel pointers
//
//
// Only for fw 9.0, for now...
// 
void init_kernel()
{
    if (kernel_base) return;

    kernel_base = load_kernel_base();
    sysents     = (struct sysent*) &kernel_base[sysent_offset]; // load syscall table
    kernel_vmmap    = (vm_map_t) &kernel_base[kkernel_map_offset]; // load vm_map used for memory alloc/free operations by the kernel
    //
    // Load kernel functions
    // 
    critical_enter  = (void(*)()) &kernel_base[critical_enter_offset];
    critical_exit   = (void(*)()) &kernel_base[critical_exit_offset];
	kprintf         = (int (*)(const char *format, ...)) (&kernel_base[kprintf_offset]);
    ksock_create    = (int (*)(void **socket, int domain, int type, int protocol)) &kernel_base[ksock_create_offset];
    ksock_close     = (int (*)(void *socket)) &kernel_base[ksock_close_offset];
    ksock_bind      = (int (*)(void* socket, struct sockaddr *addr)) &kernel_base[ksock_bind_offset];
    ksock_recv      = (int (*)(void *socket, void *buf, size_t *len)) &kernel_base[ksock_recv_offset];
    kproc_create    = (int (*)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...)) &kernel_base[kproc_create_offset];
    kmem_alloc      = (uint8_t*(*)(vm_map_t map, size_t size)) &kernel_base[kmem_alloc_offset];
    kmem_free       = (void(*)(vm_map_t map, void* addr, size_t size)) &kernel_base[kmem_free_offset];
    //
    // Load syscalls
    //
    ksys_socket      =  (int(*)(struct thread* td, struct socket_args* uap)) sysents[SYS_socket].sy_call;
    ksys_bind        =  (int(*)(struct thread* td, struct bind_args* uap)) sysents[SYS_bind].sy_call;
    ksys_recvfrom    =  (int(*)(struct thread* td, struct recvfrom_args* uap)) sysents[SYS_recvfrom].sy_call;
    ksys_close       =  (int(*)(struct thread* td, struct close_args* uap)) sysents[SYS_close].sy_call;
    ksys_read        =  (int(*)(struct thread* td, struct read_args* uap)) sysents[SYS_read].sy_call;
    ksys_listen      =  (int(*)(struct thread* td, struct listen_args* uap)) sysents[SYS_listen].sy_call;
    ksys_accept      =  (int(*)(struct thread* td, struct accept_args* uap)) sysents[SYS_accept].sy_call;
}


uint8_t* load_kernel_base()
{
    return &((uint8_t *)__readmsr(MSR_LSTAR))[-xfast_syscall_offset];  
}


//
// kalloc & kfree inspired in the Mira code
//

uint64_t* kalloc(size_t alloc_size)
{
    // give one size extra, to write the alloc_size
    alloc_size += sizeof(uint64_t);
    kprintf("Calling kmem_alloc at %llx size: %d\n", kmem_alloc, alloc_size);
    uint64_t* buff = (uint64_t*) kmem_alloc(kernel_vmmap, alloc_size);
    
    if (!buff)
    {
        return 0;
    }
    kprintf("Allocated at %llx\nApplying size...\n", buff);
    //
    // Write the alloc size
    //
    *buff++ = alloc_size; 
    kprintf("Returning...\n");

    return buff; 
}

void kfree(uint64_t* data)
{
    if (!data) return;

    //
    // Subtract the pointer by one, now it points to the real begin, which also holds the allocation size
    //
    data--;
    //
    // free it
    //
    kmem_free(kernel_vmmap, data, *data);
}

//
// Make the thread enter in a critical region
// Clear IF bit, to avoid maskable interruptions manipulating system structures/data
// Clear the wp bit
//
void enable_safe_patch()
{
    if (!kernel_base)
    {
        init_kernel();
    }

    critical_enter();
    __asm__("cli"); 
    disable_cr0_wp();
}

//
// Disable/enable everything that was done in the enable_safe_patch
//
void disable_safe_patch()
{
    if (!kernel_base)
    {
        init_kernel();
    }

    enable_cr0_wp();
    __asm__("sti");
    critical_exit();
}


void disable_cr0_wp()
{
    __writecr0(__readcr0() & ~X86_CR0_WP);
}	

void enable_cr0_wp()
{
    __writecr0(__readcr0() | X86_CR0_WP);
}