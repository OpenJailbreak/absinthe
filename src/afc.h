/*
 * afc.h
 * Functions for lockdownd's apple file conduit service
 *
 *  Created on: May 5, 2011
 *      Author: posixninja
 */

#ifndef AFC_H_
#define AFC_H_

#include <libimobiledevice/afc.h>
#include "lockdown.h"

typedef struct afc_t {
	uint16_t port;
	device_t* device;
	afc_client_t client;
} afc_t;

afc_t* afc_create();
afc_t* afc_connect(device_t* device);
afc_t* afc_open(device_t* device, uint16_t port);

int afc_send_file(afc_t* afc, const char* local, const char* remote);
int afc_close(afc_t* afc);
void afc_free(afc_t* afc);
void apparition_afc_get_file_contents(afc_t* afc, const char *filename, char **data, uint64_t *size);
#endif /* AFC_H_ */
