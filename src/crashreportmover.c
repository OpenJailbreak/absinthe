/*
 * crashreportmover.c
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#include <stdlib.h>
#include <stdio.h>
#include "crashreportmover.h"
#include "device.h"

crashreportmover_t* crashreportermover_create() {
	crashreportmover_t* mover = NULL;
	if(mover != NULL) {
		memset(mover, '\0', sizeof(mover));
	}
	return mover;
}

crashreportmover_t* crashreportmover_connect(device_t* device) {
	return NULL;
}

crashreportmover_t* crashreportermover_open(device_t* device, uint16_t port) {
	int err = 0;
	crashreportmover_t* mover = crashreportermover_create();

	err = idevice_connect(device->client, port, &(mover->connection));
	if(err < 0) {
		return NULL;
	}
	idevice_disconnect(mover->connection);
	return mover;
}



int crashreportermover_close(crashreportmover_t* mover) {
	return -1;
}

void crashreportermover_free(crashreportmover_t* mover) {
}
