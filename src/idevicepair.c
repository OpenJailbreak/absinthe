/*
 * idevicepair.c
 * Simple utility to pair/unpair an iDevice
 *
 * Copyright (c) 2010 Nikias Bassen All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "idevicepair.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

int idevicepair() {
	char *uuid = NULL;
	lockdownd_client_t client = NULL;
	idevice_t phone = NULL;
	idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
	lockdownd_error_t lerr;
	int result;

	char *type = NULL;
	char *cmd;

	if (uuid) {
		ret = idevice_new(&phone, uuid);
		free(uuid);
		uuid = NULL;
		if (ret != IDEVICE_E_SUCCESS) {
			printf("No device found with uuid %s, is it plugged in?\n", uuid);
			return EXIT_FAILURE;
		}
	} else {
		ret = idevice_new(&phone, NULL);
		if (ret != IDEVICE_E_SUCCESS) {
			printf("No device found, is it plugged in?\n");
			return EXIT_FAILURE;
		}
	}

	lerr = lockdownd_client_new(phone, &client, "idevicepair");
	if (lerr != LOCKDOWN_E_SUCCESS) {
		idevice_free(phone);
		printf("ERROR: lockdownd_client_new failed with error code %d\n", lerr);
		return EXIT_FAILURE;
	}

	result = EXIT_SUCCESS;

	lerr = lockdownd_query_type(client, &type);
	if (lerr != LOCKDOWN_E_SUCCESS) {
		printf("QueryType failed, error code %d\n", lerr);
		result = EXIT_FAILURE;
		goto leave;
	} else {
		if (strcmp("com.apple.mobile.lockdown", type)) {
			printf("WARNING: QueryType request returned '%s'\n", type);
		}
		if (type) {
			free(type);
		}
	}

	ret = idevice_get_uuid(phone, &uuid);
	if (ret != IDEVICE_E_SUCCESS) {
		printf("ERROR: Could not get device uuid, error code %d\n", ret);
		result = EXIT_FAILURE;
		goto leave;
	}

	lerr = lockdownd_pair(client, NULL);
	if (lerr == LOCKDOWN_E_SUCCESS) {
		printf("SUCCESS: Paired with device %s\n", uuid);
	} else {
		result = EXIT_FAILURE;
		if (lerr == LOCKDOWN_E_PASSWORD_PROTECTED) {
			printf(
					"ERROR: Could not pair with the device because a passcode is set. Please enter the passcode on the device and retry.\n");
		} else {
			printf(
					"ERROR: Pairing with device %s failed with unhandled error code %d\n",
					uuid, lerr);
		}
	}

	leave: if (client)
		lockdownd_client_free(client);
	if (phone)
		idevice_free(phone);
	if (uuid)
		free(uuid);
	return result;
}

int idevicevalidate() {
	char *uuid = NULL;
	lockdownd_client_t client = NULL;
	idevice_t phone = NULL;
	idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
	lockdownd_error_t lerr;
	int result;

	char *type = NULL;
	char *cmd;

	if (uuid) {
		ret = idevice_new(&phone, uuid);
		free(uuid);
		uuid = NULL;
		if (ret != IDEVICE_E_SUCCESS) {
			printf("No device found with uuid %s, is it plugged in?\n", uuid);
			return EXIT_FAILURE;
		}
	} else {
		ret = idevice_new(&phone, NULL);
		if (ret != IDEVICE_E_SUCCESS) {
			printf("No device found, is it plugged in?\n");
			return EXIT_FAILURE;
		}
	}

	lerr = lockdownd_client_new(phone, &client, "idevicepair");
	if (lerr != LOCKDOWN_E_SUCCESS) {
		idevice_free(phone);
		printf("ERROR: lockdownd_client_new failed with error code %d\n", lerr);
		return EXIT_FAILURE;
	}

	result = EXIT_SUCCESS;

	lerr = lockdownd_query_type(client, &type);
	if (lerr != LOCKDOWN_E_SUCCESS) {
		printf("QueryType failed, error code %d\n", lerr);
		result = EXIT_FAILURE;
		goto leave;
	} else {
		if (strcmp("com.apple.mobile.lockdown", type)) {
			printf("WARNING: QueryType request returned '%s'\n", type);
		}
		if (type) {
			free(type);
		}
	}

	ret = idevice_get_uuid(phone, &uuid);
	if (ret != IDEVICE_E_SUCCESS) {
		printf("ERROR: Could not get device uuid, error code %d\n", ret);
		result = EXIT_FAILURE;
		goto leave;
	}

	lerr = lockdownd_validate_pair(client, NULL);
	if (lerr == LOCKDOWN_E_SUCCESS) {
		printf("SUCCESS: Validated pairing with device %s\n", uuid);
	} else {
		result = EXIT_FAILURE;
		if (lerr == LOCKDOWN_E_PASSWORD_PROTECTED) {
			printf(
					"ERROR: Could not validate with the device because a passcode is set. Please enter the passcode on the device and retry.\n");
		} else if (lerr == LOCKDOWN_E_INVALID_HOST_ID) {
			printf("ERROR: Device %s is not paired with this host\n", uuid);
		} else {
			printf("ERROR: Pairing failed with unhandled error code %d\n",
					lerr);
		}
	}

	leave: if (client)
		lockdownd_client_free(client);
	if (phone)
		idevice_free(phone);
	if (uuid)
		free(uuid);
	return result;
}
