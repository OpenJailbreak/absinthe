/**
 * GreenPois0n Absinthe - device.c
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
#include <libimobiledevice/libimobiledevice.h>

#include "device.h"

#ifdef WIN32
#include <windows.h>
#define sleep(x) Sleep(x*1000)
#endif

device_t* device_create(const char* udid) {
	idevice_error_t err = 0;
	device_t* device = NULL;
	device = (device_t*) malloc(sizeof(device_t));
	if (device == NULL) {
		return NULL;
	}
	memset(device, '\0', sizeof(device_t));

	if (udid == NULL) {
		err = idevice_new(&(device->client), NULL);
		if (err != IDEVICE_E_SUCCESS) {
			fprintf(stderr,
					"No device found with udid %s, is it plugged in?\n", udid);
			return NULL;
		}
		idevice_get_udid(device->client, (char**)&device->udid);

	} else {
		int retries = 5;
		do {
			err = idevice_new(&(device->client), udid);
			if (device->client) {
				break;
			}
			sleep(1);
		} while (retries-- >= 0);
		if (err != IDEVICE_E_SUCCESS) {
			fprintf(stderr,
					"No device found with udid %s, is it plugged in?\n", udid);
			return NULL;
		}
		device->udid = strdup(udid);
	}

	return device;
}

void device_free(device_t* device) {
	if (device) {
		if(device->udid) {
			free(device->udid);
			device->udid = NULL;
		}
		if (device->client) {
			idevice_free(device->client);
		}
		free(device);
	}
}

void device_enable_debug() {
	idevice_set_debug_level(3);
}
