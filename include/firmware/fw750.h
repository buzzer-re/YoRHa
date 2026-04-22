#pragma once

#define kprintf_offset                  0x0026F740
#define critical_enter_offset           0x00347850
#define critical_exit_offset            0x00347870
#define xfast_syscall_offset            0x000001C0
#define sysent_offset                   0x01122340
#define ksock_create_offset             0x00321DA0
#define ksock_close_offset              0x00321E10
#define ksock_bind_offset               0x00321E20
#define ksock_recv_offset               0x00322180
#define kproc_create_offset             0x0000D8F0
#define kmem_alloc_offset               0x001753E0
#define kmem_free_offset                0x001755B0
#define kkernel_map_offset              0x021405B8
#define kmtx_destroy                    0x00310620
#define kmtx_init_offset                0x003105B0
#define kmtx_lock_sleep                 0x0030FBB0
#define kmtx_lock_spin_flags_offset     0x0030FF30
#define kmtx_unlock_sleep_offset        0x0030FED0
#define kgeneric_stop_cpus_offset       0x0026C940
#define krestart_cpus_offset            0x0026CA80
#define kmalloc_offset                  0x001D6680
#define kfree_offset                    0x001D6870
#define kcopyin_offset                  0x0028F9F0
#define kpmap_extract_offset            0x001A70D0
#define KMEM_TEMP_offset                0x01556DA0

// Kernel global variables
#define all_cpus_offset                 0x021792D0
#define stopped_cpus_offset             0x02179280