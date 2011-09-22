/*
 * crashreportmover.h
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#ifndef CRASHREPORTMOVER_H_
#define CRASHREPORTMOVER_H_

#include <libimobiledevice/libimobiledevice.h>

typedef struct crashreportmover_t {
	uint16_t port;
	device_t* device;
	idevice_connection_t connection;
} crashreportmover_t;

crashreportmover_t* crashreportmover_create();
crashreportmover_t* crashreportmover_connect(device_t* device);
crashreportmover_t* crashreportmover_open(device_t* device, uint16_t port);

int crashreportmover_close(crashreportmover_t* mover);
void crashreportmover_free(crashreportmover_t* mover);

#endif /* CRASHREPORTMOVER_H_ */
