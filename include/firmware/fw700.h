#pragma once

#define kprintf_offset                  0x000BC730
#define critical_enter_offset           0x003832B0
#define critical_exit_offset            0x003832D0
#define xfast_syscall_offset            0x000001C0
#define sysent_offset                   0x01125660
#define ksock_create_offset             0x00140610
#define ksock_close_offset              0x00140680
#define ksock_bind_offset               0x00140690
#define ksock_recv_offset               0x001409F0
#define kproc_create_offset             0x000C4170
#define kmem_alloc_offset               0x001170F0
#define kmem_free_offset                0x001172C0
#define kkernel_map_offset              0x021C8EE0
#define kmtx_destroy                    0x000BBB80
#define kmtx_init_offset                0x000BBB10
#define kmtx_lock_sleep                 0x000BB100
#define kmtx_lock_spin_flags_offset     0x000BB490
#define kmtx_unlock_sleep_offset        0x000BB430
#define kgeneric_stop_cpus_offset       0x004441F0
#define krestart_cpus_offset            0x00444340
#define kmalloc_offset                  0x00301840              
#define kfree_offset                    0x00301A40     
#define kcopyin_offset                  0x0002F230
#define kpmap_extract_offset            0x003DF0A0
#define KMEM_TEMP_offset                0x01A7AE50

// Kernel global variables
#define all_cpus_offset                 0x022D0DF0
#define stopped_cpus_offset             0x022D0DA0

