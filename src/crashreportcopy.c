/*
 * crashreportcopy.c
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>

#include "debug.h"
#include "debug.h"
#include "lockdown.h"
#include "crashreportcopy.h"


crashreportcopy_t* crashreportcopy_create() {
	crashreportcopy_t* copier = (crashreportcopy_t*) malloc(sizeof(crashreportcopy_t));
	if(copier != NULL) {
		memset(copier, '\0', sizeof(copier));
	}
	return copier;
}

crashreportcopy_t* crashreportcopy_connect(device_t* device) {
	int err = 0;
	uint16_t port = 0;
	crashreportcopy_t* copier = NULL;
	lockdown_t* lockdown = NULL;

	lockdown = lockdown_open(device);
	if(lockdown == NULL) {
		error("Unable to open connection to lockdownd\n");
		return NULL;
	}

	err = lockdown_start_service(lockdown, "com.apple.crashreportcopy", &port);
	if(err < 0) {
		error("Unable to start AFC service\n");
		return NULL;
	}
	lockdown_close(lockdown);

	copier = crashreportcopy_open(device, port);
	if(copier == NULL) {
		error("Unable to open connection to CrashReporter copy service\n");
		return NULL;
	}

	return copier;
}

crashreportcopy_t* crashreportcopy_open(device_t* device, uint16_t port) {
	int err = 0;
	crashreportcopy_t* copier = crashreportcopy_create();

	err = idevice_connect(device->client, port, &(copier->connection));
	if(err < 0) {
		return NULL;
	}
	idevice_disconnect(copier->connection);
	return copier;
}

int crashreportcopy_close(crashreportcopy_t* copier) {
	return -1;
}

void crashreportcopy_free(crashreportcopy_t* copier) {
}
