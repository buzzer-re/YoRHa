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