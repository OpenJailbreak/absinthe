/*
 * crashreportcopy.h
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#ifndef CRASHREPORTCOPY_H_
#define CRASHREPORTCOPY_H_

#include <libimobiledevice/libimobiledevice.h>

#include "lockdown.h"

typedef struct crashreportcopy_t {
	uint16_t port;
	device_t* device;
	idevice_connection_t connection;
} crashreportcopy_t;

crashreportcopy_t* crashreportcopy_create();
crashreportcopy_t* crashreportcopy_connect(device_t* device);
crashreportcopy_t* crashreportcopy_open(device_t* device, uint16_t port);

int crashreportcopy_close(crashreportcopy_t* copier);
void crashreportcopy_free(crashreportcopy_t* copier);

#endif /* CRASHREPORTCOPY_H_ */
