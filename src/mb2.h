/**
 * GreenPois0n Absinthe - mb2.h
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

#ifndef MB2_H_
#define MB2_H_

#include <libimobiledevice/mobilebackup2.h>

#include "device.h"
#include "lockdown.h"

typedef struct mb2_t {
	uint16_t port;
	device_t* device;
	mobilebackup2_client_t client;
	unsigned char *poison;
	size_t poison_length;
	int poison_spilled;
} mb2_t;

#define CODE_SUCCESS 0x00
#define CODE_ERROR_LOCAL 0x06
#define CODE_ERROR_REMOTE 0x0b
#define CODE_FILE_DATA 0x0c

mb2_t* mb2_create();
mb2_t* mb2_connect(device_t* device);
mb2_t* mb2_open(device_t* device, uint16_t port);
void mb2_free(mb2_t* mb2);

int mb2_crash(mb2_t* mb2);
int mb2_inject(mb2_t* mb2, char* data, int size);
int mb2_exploit(mb2_t* mb2);

/* Status.plist callback function prototype */
/* NOTE: if newplist is to be replaced with a pointer to a new plist,
   use plist_free() to free it first. */
typedef void (*mb2_status_plist_cb_t)(plist_t *newplist, void *userdata);

/* attack string callback function prototype */
/* NOTE: newplist will be passed as NULL */
typedef void (*mb2_attack_plist_cb_t)(plist_t *newplist, void *userdata);

void mb2_set_status_plist_cb_func(mb2_status_plist_cb_t callback, void *userdata);

void mb2_set_attack_plist_cb_func(mb2_attack_plist_cb_t callback, void *userdata);

#endif /* MB2_H_ */
