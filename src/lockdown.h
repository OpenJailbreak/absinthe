/*
 * lockdown.h
 *
 *  Created on: May 5, 2011
 *      Author: posixninja
 */

#ifndef LOCKDOWN_H_
#define LOCKDOWN_H_

#include <libimobiledevice/lockdown.h>
#include "device.h"

typedef struct lockdown_t {
	device_t *device;
	lockdownd_client_t client;
} lockdown_t;

lockdown_t* lockdown_open(device_t* device);
int lockdown_start_service(lockdown_t* lockdown, const char* service, uint16_t* port);
int lockdown_stop_service(lockdown_t* lockdown, const char* service);
int lockdown_close(lockdown_t* lockdown);
void lockdown_free(lockdown_t* lockdown);

#endif /* LOCKDOWN_H_ */
