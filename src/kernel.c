#include "../include/kernel.h"

void (*critical_enter)();
void (*critical_exit)();
int (*kprintf)(const char *format, ...) = (int(*)(const char *, ...)) NULL;
int (*ksock_create)(void **socket, int domain, int type, int protocol);
int (*ksock_close)(void *socket);
int (*ksock_bind)(void *socket, struct sockaddr *addr);
int (*ksock_recv)(void *socket, void *buf, size_t *len);
int (*kproc_create)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...);
void (*kmtx_init)(struct mtx *m, const char *name, const char *type, int opts);
int (*kgeneric_stop_cpus)(cpuset_t map, uint32_t type);
int (*krestart_cpus)(cpuset_t map);
void* (*kmalloc)(unsigned long size, struct malloc_type *mtp, int flags);
void (*kfree)(void* addr, struct malloc_type *mtp);
int (*kcopyin)(const void *src, void *dst, size_t len);


vm_paddr_t (*kpmap_kextract)(vm_offset_t* va);

//
// Syscalls
//
int (*ksys_socket)(struct thread* td, struct socket_args* uap);
int (*ksys_bind)(struct thread* td, struct bind_args* uap);
int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
int (*ksys_close)(struct thread* td, struct close_args* uap);
int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
int (*ksys_read)(struct thread* td, struct read_args* uap);
int (*ksys_accept)(struct thread* td, struct accept_args* uap);
int (*ksys_listen)(struct thread* td, struct listen_args* uap);
int (*ksys_sendto)(struct thread* td, struct sendto_args* uap);
int (*ksys_fcntl)(struct thread* td, struct fcntl_args* uap);
int (*ksys_select)(struct thread* td, struct select_args* uap);
int (*ksys_shutdown)(struct thread* td, struct shutdown_args* uap);
int (*ksys_setsockopt)(struct thread* td, struct setsockopt_args* uap);

vm_offset_t* (*kmem_alloc)(vm_map_t map, size_t size);
void (*kmem_free)(vm_map_t map, void* addr, size_t size);

uint8_t* kernel_base = 0;
struct sysent* sysents;
struct malloc_type* KM_TEMP;


//
// Kernel memory structs
//

vm_map_t kernel_vmmap;

//
// Init kernel fptrs and syscalls
//
// 
void init_kernel()
{
    if (kernel_base) return;

    //
    // Load kernel variables
    //
    kernel_base     = load_kernel_base();
    sysents         = (struct sysent*) &kernel_base[sysent_offset]; // load syscall table
    kernel_vmmap    = (vm_map_t) (*(uint64_t*)&kernel_base[kkernel_map_offset]); // load vm_map used for memory alloc/free operations in the kernel space
    KM_TEMP          = (struct malloc_type*) &kernel_base[KMEM_TEMP_offset]; // malloc temporary data buffers 
    //
    // Load kernel functions
    // 
    critical_enter      = (void(*)()) &kernel_base[critical_enter_offset];
    critical_exit       = (void(*)()) &kernel_base[critical_exit_offset];
	kprintf             = (int (*)(const char *format, ...)) (&kernel_base[kprintf_offset]);
    ksock_create        = (int (*)(void **socket, int domain, int type, int protocol)) &kernel_base[ksock_create_offset];
    ksock_close         = (int (*)(void *socket)) &kernel_base[ksock_close_offset];
    ksock_bind          = (int (*)(void* socket, struct sockaddr *addr)) &kernel_base[ksock_bind_offset];
    ksock_recv          = (int (*)(void *socket, void *buf, size_t *len)) &kernel_base[ksock_recv_offset];
    kproc_create        = (int (*)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...)) &kernel_base[kproc_create_offset];
    kmem_alloc          = (vm_offset_t*(*)(vm_map_t map, size_t size)) &kernel_base[kmem_alloc_offset];
    kmem_free           = (void(*)(vm_map_t map, void* addr, size_t size)) &kernel_base[kmem_free_offset];
    kmtx_init           = (void(*)(struct mtx *m, const char *name, const char *type, int opts)) &kernel_base[kmtx_init_offset];
    kgeneric_stop_cpus  = (int (*)(cpuset_t map, uint32_t type)) &kernel_base[kgeneric_stop_cpus_offset];
    krestart_cpus       = (int(*)(cpuset_t map)) &kernel_base[krestart_cpus_offset];
    kmalloc             = (int(*)(unsigned long size, struct malloc_type* mtp, int flags )) &kernel_base[kmalloc_offset];
    kfree               = (void(*)(void* addr, struct malloc_type* mtp)) &kernel_base[kfree_offset];
    kpmap_kextract      = (vm_paddr_t(*)(void* va)) &kernel_base[kpmap_extract_offset];
    kcopyin             = (int(*)(const void *src, void *dst, size_t len)) &kernel_base[kcopyin_offset];
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
    ksys_sendto      =  (int(*)(struct thread* td, struct sendto_args* uap)) sysents[SYS_sendto].sy_call;
    ksys_fcntl       =  (int(*)(struct thread* td, struct fcntl_args* uap)) sysents[SYS_fcntl].sy_call;
    ksys_select      =  (int(*)(struct thread* td, struct select_args* uap)) sysents[SYS_select].sy_call;
    ksys_shutdown    =  (int(*)(struct thread* td, struct shutdown_args* uap)) sysents[SYS_shutdown].sy_call;
    ksys_setsockopt  =  (int(*)(struct thread* td, struct setsockopt_args* uap)) sysents[SYS_setsockopt].sy_call;
}


uint8_t* load_kernel_base()
{
    return &((uint8_t *)__readmsr(MSR_LSTAR))[-xfast_syscall_offset];  
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