#include "config/iOS/5.0.1/iPhone3,1/constants.h"

#ifndef CONSTANTS_SET
#define STRLEN_PID		4
#define STRLEN_FILENAME		66
#define LOG_SHIFT		(16 + STRLEN_PID + STRLEN_FILENAME)

#define DEBUG
#define P1				402
#define P2				619
#define P3				625
//   p2 address: 0xXXXXXX[2c]
#define P3_ADDR_LO			0x2c + (P3 - P2) * 4;
#define LR_ADDR_LO			0x30
#endif

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
