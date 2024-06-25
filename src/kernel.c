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

uint8_t* kernel_base = 0;

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
    //
    // Load kernel functions, syscalls and so on
    // 
    critical_enter  = (void(*)()) &kernel_base[critical_enter_offset]; // critical_enter offset, from mira
    critical_exit   = (void(*)()) &kernel_base[critical_exit_offset]; // critical_exit offset, from mira
	kprintf         = (int (*)(const char *format, ...)) (&kernel_base[kprintf_offset]);
    ksock_create    = (int (*)(void **socket, int domain, int type, int protocol)) &kernel_base[ksock_create_offset];
    ksock_close     = (int (*)(void *socket)) &kernel_base[ksock_close_offset];
    ksock_bind      = (int (*)(void* socket, struct sockaddr *addr)) &kernel_base[ksock_bind_offset];
    ksock_recv      = (int (*)(void *socket, void *buf, size_t *len)) &kernel_base[ksock_recv_offset];
    kproc_create    = (int (*)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...)) &kernel_base[kproc_create_offset];

    //
    // Load syscalls
    //
    ksys_socket      =  (int(*)(struct thread* td, struct socket_args* uap)) sysents[SYS_socket].sy_call;
    ksys_bind        =  (int(*)(struct thread* td, struct bind_args* uap)) sysents[SYS_bind].sy_call;
    ksys_recvfrom    =  (int(*)(struct thread* td, struct recvfrom_args* uap)) sysents[SYS_recvfrom].sy_call;
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