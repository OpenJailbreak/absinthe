/**
 * GreenPois0n Absinthe - crashreporter.c
 * Copyright (C) 2010 Chronic-Dev Team
 * Copyright (C) 2010 Joshua Hill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "debug.h"
#include "lockdown.h"
#include "crashreporter.h"
#include "crashreportcopy.h"
#include "crashreportmover.h"

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
	if(crashreporter != NULL) {
		crashreporter->mover = crashreportmover_connect(device);
		if(crashreporter->mover == NULL) {
			error("Unable to connect to CrashReporter's mover service\n");
			return NULL;
		}

		crashreporter->copier = crashreportcopy_connect(device);
		if(crashreporter->copier == NULL) {
			error("Unable to connect to CrashReporter's copier service\n");
			return NULL;
		}
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
	crashreportmover_t* mover = crashreportmover_open(device, port);
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
			crashreportmover_free(crashreporter->mover);
		}
		if (crashreporter->copier) {
			crashreportcopy_free(crashreporter->copier);
		}
		free(crashreporter);
	}
}


crashreport_t* crashreporter_last_crash(crashreporter_t* crashreporter) {
	return NULL;
}
