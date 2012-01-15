#include "config/iOS/5.0.1/iPhone4,1/constants.h"

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
