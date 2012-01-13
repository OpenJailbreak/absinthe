/*
 * constants.h
 *
 *  Created on: Jan 12, 2012
 *      Author: posixninja
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "config/iOS/5.0.1/iPhone4,1/constants.h"

#define STRLEN_PID		4
#define STRLEN_FILENAME		66
#define LOG_SHIFT		(16 + STRLEN_PID + STRLEN_FILENAME)

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


#endif /* CONSTANTS_H_ */
