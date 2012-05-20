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
#include <libimobiledevice/libimobiledevice.h>

#include "debug.h"
#include "common.h"
#include "device.h"
#include "lockdown.h"

lockdown_t* lockdown_open(device_t* device) {
	lockdownd_client_t lockdownd = NULL;
	if (lockdownd_client_new_with_handshake(device->client, &lockdownd,
			"absinthe") != LOCKDOWN_E_SUCCESS) {
		error("Unable to pair with lockdownd\n");
		return NULL;
	}

	lockdown_t* lockdown = (lockdown_t*) malloc(sizeof(lockdown_t));
	if (lockdown == NULL) {
		error("Unable to allocate memory for lockdown object\n");
		return NULL;
	}
	memset(lockdown, '\0', sizeof(lockdown_t));

	lockdown->client = lockdownd;
	lockdown->device = device;
	return lockdown;
}

int lockdown_get_value(lockdown_t* lockdown, const char *domain,
		const char *key, plist_t *value) {
	if (!lockdown || !lockdown->client) {
		return -1;
	}

	lockdownd_error_t err = lockdownd_get_value(lockdown->client, domain, key,
			value);
	if (err == LOCKDOWN_E_SUCCESS) {
		return 0;
	} else {
		return -1;
	}
}

int lockdown_get_string(lockdown_t* lockdown, const char *key, char** value) {
	if (!lockdown || !lockdown->client) {
		return -1;
	}

	char* str = NULL;
	plist_t pl = NULL;
	lockdownd_error_t err = lockdownd_get_value(lockdown->client, NULL, key, &pl);
	if (err == LOCKDOWN_E_SUCCESS) {
		if (pl != NULL && plist_get_node_type(pl) == PLIST_STRING) {
			plist_get_string_val(pl, &str);
			plist_free(pl);
			if (str != NULL) {
				*value = str;
				return 0;
			}
		}
	}
	if (pl) {
		plist_free(pl);
	}
	return -1;
}

int lockdown_start_service(lockdown_t* lockdown, const char* service,
		uint16_t* port) {
    return lockdown_start_service2(lockdown, service, port, 1);
}

int lockdown_start_service2(lockdown_t* lockdown, const char* service,
		uint16_t* port, int warn_on_fail) {
	uint16_t p = 0;
	lockdownd_start_service(lockdown->client, service, &p);

	if (p == 0) {
		if(warn_on_fail) error("%s failed to start!\n", service);
		return -1;
	}

	debug("Started %s successfully on port %d!\n", service, p);
	*port = p;
	return 0;
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
