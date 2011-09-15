/*
 * mb2.c
 *
 *  Created on: May 5, 2011
 *      Author: posixninja
 */

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
	return mb2_create();
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
