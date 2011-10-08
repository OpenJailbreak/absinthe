/**
 * GreenPois0n Absinthe - crashreporter.h
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

#ifndef CRASHREPORTER_H_
#define CRASHREPORTER_H_

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/afc.h>
#include <plist/plist.h>

#include "afc.h"
#include "crashreport.h"
#include "crashreportcopy.h"
#include "crashreportmover.h"

typedef struct crashreporter_t {
	//afc_t* afc;
	uint16_t port;
	device_t* device;
	crashreportcopy_t* copier;
	crashreportmover_t* mover;
} crashreporter_t;

crashreporter_t* crashreporter_create();
crashreporter_t* crashreporter_connect(device_t* device);
crashreporter_t* crashreporter_open(device_t* device, uint16_t port);
void crashreporter_free(crashreporter_t* crashreporter);

crashreport_t* crashreporter_last_crash(crashreporter_t* crashreporter);

#endif
