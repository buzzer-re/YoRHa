#pragma once

#define kprintf_offset          0x000B7A30
#define critical_enter_offset   0x002C1980
#define critical_exit_offset    0x002C19A0
#define xfast_syscall_offset    0x000001C0
#define sysent_offset           0x01100310
#define ksock_create_offset     0x0041bd20
#define ksock_close_offset      0x0041bd90
#define ksock_bind_offset       0x0041bda0
#define ksock_recv_offset       0x0041c100
#define kproc_create_offset     0x000969e0
#define kmem_alloc_offset       0x0037BE70
#define kmem_free_offset        0x0037C040
#define kkernel_map_offset      0x02268d48
#define kmtx_destroy            0x002EF9D0
#define kmtx_init_offset        0x002EF960
#define kmtx_lock_sleep         0x002EEF50
#define kmtx_unlock_sleep       0x002EF270