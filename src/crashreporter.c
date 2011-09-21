/*
 *  crashreporter.c
 *  apparition
 *
 *  Created by posixninja on 5/25/11.
 *  Copyright 2011 Chronic-Dev, LLC All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "lockdown.h"
#include "crashreporter.h"

crashreporter_t* crashreporter_create() {
	crashreporter_t* crashreporter = (crashreporter_t*) malloc(sizeof(crashreporter_t));
	if(crashreporter) {
		memset(crashreporter, '\0', sizeof(crashreporter_t));
	}
	return crashreporter;
}

crashreporter_t* crashreporter_connect(device_t* device) {
	int err = 0;
	uint16_t port = 0;
	crashreporter_t* crashreporter = crashreporter_create();

	crashreporter->mover = crashreportmover_connect(device);
	if(crashreporter->mover == NULL) {
		error("Unable to connect to CrashReporter's mover service\n");
		return NULL;
	}

	crashreporter->copier = crashreportcopier_connect(device);
	if(crashreporter->copier == NULL) {
		error("Unable to connect to CrashReporter's copier service\n");
		return NULL;
	}

	return crashreporter;
}

crashreporter_t* crashreporter_open(device_t* device, uint16_t port) {
	crashreporter_t* crashreporter = crashreporter_create();
	if(crashreporter == NULL) {
		error("Unable to create CrashReporter client\n");
		return NULL;
	}
	// Startup crashreportmover service to move our crashes to the proper place ???
	crashreportmover_t* mover = crashreportermover_open(device, port);
	if(mover == NULL) {
		
		printf("failed to open crashreportermover_open!\n");
		
		return NULL;
	}
	
	// Startup crashreporter copy to copy them to mobile root??	
	crashreportcopy_t* copier = crashreportcopy_open(device, port);
	if(copier == NULL) {
		//crashreportmover_free(mover);
		return NULL;
	}
	
	crashreporter->mover = mover;
	crashreporter->copier = copier;
	crashreporter->device = device;

	return crashreporter;
}

int crashreporter_close(crashreporter_t* crashreporter) {
	return -1;
}

void crashreporter_free(crashreporter_t* crashreporter) {
	if (crashreporter) {
		if (crashreporter->mover) {
			crashreportermover_free(crashreporter->mover);
		}
		if (crashreporter->copier) {
			crashreportcopy_free(crashreporter->copier);
		}
		free(crashreporter);
	}
}
