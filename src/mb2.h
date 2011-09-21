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
} mb2_t;

mb2_t* mb2_create();
mb2_t* mb2_connect(device_t* device);
mb2_t* mb2_open(device_t* device, uint16_t port);
void mb2_free(mb2_t* mb2);

int mb2_crash(mb2_t* mb2);
int mb2_inject(mb2_t* mb2);
int mb2_exploit(mb2_t* mb2);

#endif /* MB2_H_ */
