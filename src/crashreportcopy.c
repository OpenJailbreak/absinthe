/*
 * crashreportcopy.c
 *
 *  Created on: May 24, 2011
 *      Author: posixninja
 */

#include <stdlib.h>
#include <stdio.h>
#include "crashreportcopy.h"
#include "device.h"
#include <string.h>

crashreportcopy_t* crashreportcopy_create() {
	crashreportcopy_t* copier = (crashreportcopy_t*) malloc(sizeof(crashreportcopy_t));
	if(copier != NULL) {
		memset(copier, '\0', sizeof(copier));
	}
	return copier;
}

crashreportcopy_t* crashreportcopy_connect(device_t* device) {
	return NULL;
}

crashreportcopy_t* crashreportcopy_open(device_t* device, uint16_t port) {
	return NULL;
}



int crashreportcopy_close(crashreportcopy_t* copier) {
	return -1;
}

void crashreportcopy_free(crashreportcopy_t* copier) {
}
