/**
 * GreenPois0n Absinthe - crashreportcopy.c
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

	err = lockdown_start_service(lockdown, "com.apple.crashreportcopymobile", &port);
	if(err < 0) {
		error("Unable to start AFC service\n");
		return NULL;
	}
	lockdown_close(lockdown);
	lockdown_free(lockdown);

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
	if(copier != NULL) {
		err = afc_client_new(device->client, port, &(copier->client));
		if(err < 0) {
			return NULL;
		}
	}
	return copier;
}

int crashreportcopy_close(crashreportcopy_t* copier) {
	afc_client_free(copier->client);
	copier->client = NULL;
	return 0;
}

void crashreportcopy_free(crashreportcopy_t* copier) {
	if(copier) {
		if(copier->client) {
			crashreportcopy_close(copier);
		}
		free(copier);
	}
}
