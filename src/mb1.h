/**
 * GreenPois0n Absinthe - mb1.h
 * Copyright (C) 2012 Chronic-Dev Team
 * Copyright (C) 2012 Nikias Bassen
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

#ifndef MB1_H_
#define MB1_H_

#include <libimobiledevice/mobilebackup.h>

#include "device.h"
#include "lockdown.h"

typedef struct mb1_t {
	uint16_t port;
	device_t* device;
	mobilebackup_client_t client;
} mb1_t;

struct mobilebackup_client_private {
	void* parent;
};

mb1_t* mb1_create();
mb1_t* mb1_connect(device_t* device);
mb1_t* mb1_open(device_t* device, uint16_t port);
void mb1_free(mb1_t* mb1);

int mb1_crash(mb1_t* mb1);

#endif /* MB1_H_ */
