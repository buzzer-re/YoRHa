#pragma once

#include "intrin.h"
#include "firmware/offset.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/sysproto.h>
#include <sys/syscall.h>
#include <sys/sysent.h>
#include <vm/vm.h>

#define DISABLE_WP() \
        __asm__("cli");\
        disable_cr0_wp()\

#define ENABLE_WP()\
        __asm__("sti");\
        enable_cr0_wp()\

void init_kernel();
uint8_t* load_kernel_base();
void enable_safe_patch();
void disable_safe_patch();
void disable_cr0_wp();
void enable_cr0_wp();
uint64_t* kalloc(size_t alloc_size);
void kfree(uint64_t* data);


extern uint8_t* kernel_base;
extern struct sysent* sysents; // syscall table


//
// Kernel functions
///
extern int (*kprintf)(const char *format, ...);
extern int (*kproc_create)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...);
extern void (*kmtx_init)(struct mtx *m, const char *name, const char *type, int opts);
extern int (*kgeneric_stop_cpus)(cpuset_t map, uint32_t type);
extern int (*krestart_cpus)(cpuset_t map);

//
// Syscalls
//
extern int (*ksys_socket)(struct thread* td, struct socket_args* uap);
extern int (*ksys_bind)(struct thread* td, struct bind_args* uap);
extern int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
extern int (*ksys_close)(struct thread* td, struct close_args* uap);
extern int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
extern int (*ksys_read)(struct thread* td, struct read_args* uap);
extern int (*ksys_accept)(struct thread* td, struct accept_args* uap);
extern int (*ksys_listen)(struct thread* td, struct listen_args* uap);
extern int (*ksys_sendto)(struct thread* td, struct sendto_args* uap);