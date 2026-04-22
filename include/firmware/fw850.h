#pragma once

#define kprintf_offset                  0x0015D570
#define critical_enter_offset           0x00417D40
#define critical_exit_offset            0x00417D60
#define xfast_syscall_offset            0x000001C0
#define sysent_offset                   0x010FC5C0
#define ksock_create_offset             0x00131600
#define ksock_close_offset              0x00131670
#define ksock_bind_offset               0x00131680
#define ksock_recv_offset               0x001319E0
#define kproc_create_offset             0x00010610
#define kmem_alloc_offset               0x002199A0
#define kmem_free_offset                0x00219B70
#define kkernel_map_offset              0x01C64228
#define kmtx_destroy                    0x0026FBF0
#define kmtx_init_offset                0x0026FB80
#define kmtx_lock_sleep                 0x0026F170
#define kmtx_lock_spin_flags_offset     0x0026F4F0
#define kmtx_unlock_sleep_offset        0x0026F490
#define kgeneric_stop_cpus_offset       0x0000F700
#define krestart_cpus_offset            0x0000F810
#define kmalloc_offset                  0x000B5A40           
#define kfree_offset                    0x000B5C00
#define kcopyin_offset                  0x003A42E0
#define kpmap_extract_offset            0x00116D80
#define KMEM_TEMP_offset                0x01528FF0

// Kernel global variables
#define all_cpus_offset                 0x01B243D0
#define stopped_cpus_offset             0x01B24380

