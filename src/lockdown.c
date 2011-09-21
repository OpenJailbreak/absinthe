/**
 * GreenPois0n Absinthe - lockdown.c
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
#include <libimobiledevice/lockdown.h>

#include "device.h"
#include "lockdown.h"

lockdown_t* lockdown_open(device_t* device) {
	lockdownd_client_t lockdownd = NULL;
	if (lockdownd_client_new_with_handshake(device->client, &lockdownd, "absinthe") != LOCKDOWN_E_SUCCESS) {
		return NULL;
	}

	lockdown_t* lockdown = (lockdown_t*) malloc(sizeof(lockdown_t));
	if (lockdown == NULL) {
		return NULL;
	}
	memset(lockdown, '\0', sizeof(lockdown_t));

	lockdown->client = lockdownd;
	lockdown->device = device;
	return lockdown;
}

int lockdown_start_service(lockdown_t* lockdown, const char* service, uint16_t* port) {
	uint16_t port_value = 0;
	lockdownd_start_service(lockdown->client, service, &port_value);

	if (port_value) {
		printf("Started %s successfully on port %d!\n", service, port_value);
		*port = port_value;
		return 0;
	} else {

		printf("%s failed to start!\n", service);
		return -1;
	}

	return -1;
}

int lockdown_stop_service(lockdown_t* lockdown, const char* service) {
	//TODO: Implement Me
	return -1;
}

int lockdown_close(lockdown_t* lockdown) {
	lockdownd_client_free(lockdown->client);
	lockdown->client = NULL;
	return 0;
}

void lockdown_free(lockdown_t* lockdown) {
	if (lockdown) {
		if (lockdown->client) {
			lockdown_close(lockdown);
		}
		free(lockdown);
	}
}
