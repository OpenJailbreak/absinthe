/*
 *  crashreporter.h
 *  apparition
 *
 *  Created by posixninja on 5/25/11.
 *  Copyright 2011 Chronic-Dev, LLC. All rights reserved.
 *
 */

#ifndef CRASHREPORTER_H_
#define CRASHREPORTER_H_

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/afc.h>
#include <plist/plist.h>

#include "afc.h"
#include "crashreportcopy.h"
#include "crashreportmover.h"

typedef struct crashreport_t {
	int i;
} crashreport_t;

typedef struct crashreporter_t {
	//afc_t* afc;
	uint16_t port;
	device_t* device;
	crashreportcopy_t* copier;
	crashreportmover_t* mover;
} crashreporter_t;

crashreporter_t* crashreporter_create();
crashreporter_t* crashreporter_connect(device_t* device);
crashreporter_t* crashreporter_open(device_t* device, uint16_t port);
void crashreporter_free(crashreporter_t* crashreporter);

crashreport_t* crashreporter_last_crash(crashreporter_t* crashreporter);

#endif
