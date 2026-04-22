#pragma once

#define kprintf_offset                  0x000FC460
#define critical_enter_offset           0x003D2280
#define critical_exit_offset            0x003D22A0
#define xfast_syscall_offset            0x000001C0
#define sysent_offset                   0x010FC4D0
#define ksock_create_offset             0x000FBF90
#define ksock_close_offset              0x000FC000
#define ksock_bind_offset               0x000FC010
#define ksock_recv_offset               0x000FC370
#define kproc_create_offset             0x0046DFD0
#define kmem_alloc_offset               0x0001B3F0
#define kmem_free_offset                0x0001B5C0
#define kkernel_map_offset              0x01B243E0
#define kmtx_destroy                    0x00298510
#define kmtx_init_offset                0x002984A0
#define kmtx_lock_sleep                 0x00297A90
#define kmtx_lock_spin_flags_offset     0x00297E10
#define kmtx_unlock_sleep_offset        0x00297DB0
#define kgeneric_stop_cpus_offset       0x0012E4F0
#define krestart_cpus_offset            0x0012E600
#define kmalloc_offset                  0x0046F7F0         
#define kfree_offset                    0x0046F9B0   
#define kcopyin_offset                  0x0025E3B0
#define kpmap_extract_offset            0x00380F20
#define KMEM_TEMP_offset                0x01A77E10

// Kernel global variables
#define all_cpus_offset                 0x0216EC70
#define stopped_cpus_offset             0x0216EC20

