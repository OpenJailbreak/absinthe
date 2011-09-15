/*
 * mb2.h
 * Functions for lockdownd's mobilebackup2 service
 *
 *  Created on: May 5, 2011
 *      Author: posixninja
 */

#ifndef MB2_H_
#define MB2_H_

#include <libimobiledevice/mobilebackup2.h>

#include "device.h"
#include "lockdown.h"

typedef struct mb2_t {
	uint16_t port;
	device_t* device;
	mobilebackup2_client_t client;
} mb2_t;

mb2_t* mb2_create();
mb2_t* mb2_connect(device_t* device);
mb2_t* mb2_open(device_t* device, uint16_t port);
void mb2_free(mb2_t* mb2);

#endif /* MB2_H_ */
