/*
 * crashreportcopy.h
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#ifndef CRASHREPORTCOPY_H_
#define CRASHREPORTCOPY_H_

#include <plist/plist.h>
#include <libimobiledevice/libimobiledevice.h>

#include "lockdown.h"

typedef struct crashreportcopy_t {
	unsigned short port;
	idevice_connection_t connection;
} crashreportcopy_t;

crashreportcopy_t* crashreportcopy_create();
crashreportcopy_t crashreport_connect(device_t* device, uint16_t port);
crashreportcopy_t* crashreportcopy_open(device_t);
int crashreportercopy_close(crashreportcopy_t* copier);
void crashreportercopy_free(crashreportcopy_t* copier);

#endif /* CRASHREPORTCOPY_H_ */
