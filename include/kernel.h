#pragma once

#include "intrin.h"
#include "firmware/offset.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sysproto.h>
#include <sys/syscall.h>
#include <sys/sysent.h>

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


extern uint8_t* kernel_base;
extern struct sysent* sysents; // syscall table


//
// Kernel functions
///
extern int (*kprintf)(const char *format, ...);
extern int (*kproc_create)(void (*func)(void *), void *arg, struct proc **newpp, int flags, int pages, const char *fmt, ...);

//
// Syscalls
//
extern int (*ksys_socket)(struct thread* td, struct socket_args* uap);
extern int (*ksys_bind)(struct thread* td, struct bind_args* uap);
extern int (*ksys_recvfrom)(struct thread* td, struct recvfrom_args* uap);
extern int (*ksys_close)(struct thread* td, struct close_args* uap);
