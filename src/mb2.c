/**
 * GreenPois0n Absinthe - mb2.c
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

#include "mb2.h"
#include "debug.h"

mb2_t* mb2_create() {
	mb2_t* mb2 = (mb2_t*) malloc(sizeof(mb2_t));
	if (mb2 != NULL) {
		memset(mb2, '\0', sizeof(mb2_t));
	}
	return mb2;
}

mb2_t* mb2_connect(device_t* device) {
	int err = 0;
	uint16_t port = 0;
	mb2_t* mb2 = NULL;
	lockdown_t* lockdown = NULL;

	lockdown = lockdown_open(device);
	if(lockdown == NULL) {
		error("Unable to open connection to lockdownd\n");
		return NULL;
	}

	err = lockdown_start_service(lockdown, "com.apple.mobilebackup2", &port);
	if(err < 0) {
		error("Unable to start MobileBackup2 service\n");
		return NULL;
	}
	lockdown_close(lockdown);

	mb2 = mb2_open(device, port);
	if(mb2 == NULL) {
		error("Unable to open connection to MobileBackup2 service\n");
		return NULL;
	}

	return mb2;
}

mb2_t* mb2_open(device_t* device, uint16_t port) {
	mobilebackup2_error_t err = MOBILEBACKUP2_E_SUCCESS;
	mb2_t* mb2 = mb2_create();
	if(mb2 != NULL) {
		err = mobilebackup2_client_new(device->client, port, &(mb2->client));
		if(err != MOBILEBACKUP2_E_SUCCESS) {
			error("Unable to create new MobileBackup2 client\n");
			mb2_free(mb2);
			return NULL;
		}
		mb2->device = device;
		mb2->port = port;
	}
	return mb2;
}

void mb2_free(mb2_t* mb2) {
	if (mb2) {
		if (mb2->client) {
			mobilebackup2_client_free(mb2->client);
			mb2->client = NULL;
		}
		mb2->device = NULL;
		free(mb2);
	}
}

// Implement these for jailbreak
int mb2_crash(mb2_t* mb2) {
	return 0;
}

int mb2_inject(mb2_t* mb2) {
	return 0;
}

int mb2_exploit(mb2_t* mb2) {
	return 0;
}
