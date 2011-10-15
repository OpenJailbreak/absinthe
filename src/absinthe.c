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

#include "mb2.h"
#include "debug.h"
#include "device.h"
#include "dictionary.h"
#include "crashreporter.h"

#include "dyldcache.h"

#include "offsets.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

/////////////////////////////////////////////////////////////////////////////////////////
/// TODO: We need to add an event handler for when devices are connected. This handler //
///         needs to wait for iTunes to autostart and kill it before it can start the  //
///         syncing process and mess up our connection.                                //
/////////////////////////////////////////////////////////////////////////////////////////

static struct option longopts[] = {
	{ "help",        no_argument,         NULL,   'h' },
	{ "verbose",     required_argument,   NULL,   'v' },
	{ "uuid",        required_argument,   NULL,   'u' },
	{ "loop",        required_argument,   NULL,   'l' },
	{ "salt",        required_argument,   NULL,   's' },
	{ "entropy",     required_argument,   NULL,   'e' },
	{ "target",      required_argument,   NULL,   't' },
	{ "pointer",     required_argument,   NULL,   'p' },
	{ "aslr-slide",  required_argument,   NULL,   'a' },
	{ "cache",       required_argument,   NULL,   'c' },
	{ "dylib",       required_argument,   NULL,   'd' },
	{ NULL, 0, NULL, 0 }
};

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

int prepare_attack() {
	return 0;
}

int bruteforce_string() {
	return 0;
}

crashreport_t* crash_mobilebackup(device_t* device) {
	crashreport_t* crash = NULL;
	mb2_t* mb2 = mb2_connect(device);
	if(mb2) {
		mb2_set_attack_plist_cb_func(&attack_plist_cb, mb2);
		mb2_crash(mb2);
		crash = fetch_crashreport(device);
	}
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
	printf("Copyright 2011, Chronic-Dev LLC\n");
	printf("Jailbreak iOS5.0 using ub3rl33t MobileBackup2 exploit.\n");
	printf("Discovered by Nikias Bassen, Exploited by Joshua Hill\n");
	printf("  General\n");
	printf("    -h, --help\t\t\tprints usage information\n");
	printf("    -v, --verbose\t\tprints debuging info while running\n");
	printf("    -u, --uuid UUID\t\ttarget specific device by its 40-digit device UUID\n");
	printf("\n  Brute Forcing\n");
	printf("    -l, --loop AMOUNT\t\tloop the attack AMOUNT number of times\n");
	printf("    -s, --salt NUMBER\t\tsalt the random number generator with this number\n");
	printf("    -e, --entropy NUMBER\tdecides how random we make our fuzzing attack\n");
	printf("\n  Payload Generation\n");
	printf("    -t, --target ADDRESS\toffset to ROP gadget we want to execute\n");
	printf("    -p, --pointer ADDRESS\theap address we're hoping contains our target\n");
	printf("    -a, --aslr-slide OFFSET\tvalue of randomized dyldcache slide\n");
	printf("    -c, --cache FILE\t\tcurrent devices dyldcache for finding ROP gadget\n");
	printf("    -d, --dylib NAME\t\tname of dylib to search for ROP gadget in\n");
	printf("\n");
}

int main(int argc, char* argv[]) {
	int opt = 0;
	int optindex = 0;

	int verbose = 0;
	char* dylib = NULL;
	char* cache = NULL;
	unsigned long aslr_slide = 0;
	unsigned long pointer = 0;
	unsigned long target = 0;
	unsigned long entropy = 0;
	unsigned long salt = 0;
	char* uuid = NULL;
	unsigned long loop = 0;

	while ((opt = getopt_long(argc, argv, "hvd:c:a:p:t:e:s:u:l:", longopts, &optindex)) > 0) {
		switch (opt) {
		case 'h':
			usage(argc, argv);
			return 0;

		case 'v':
			verbose++;
			break;

		case 'd':
			dylib = optarg;
			break;

		case 'c':
			cache = optarg;
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

		case 'e':
			entropy = strtoul(optarg, NULL, 0);
			break;

		case 's':
			salt = strtoul(optarg, NULL, 0);
			break;

		case 'u':
			uuid = optarg;
			break;

		case 'l':
			loop = strtoul(optarg, NULL, 0);
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

	// Open a connection to our device
	debug("Openning connection to device\n");
	device_t* device = device_create(uuid);
	if(device == NULL) {
		error("Unable to open device\n");
		return -1;
	}

	// Crash MobileBackup2 once so we can grab a fresh crashreport
	debug("Grabbing a fresh crashreport for this device\n");
	crashreport_t* crash = crash_mobilebackup(device);
	if(crash == NULL) {
		error("Unable to get fresh crash from mobilebackup\n");
		return -1;
	}

	// If aslr slide wasn't specified on the command line go ahead and figure it out ourself
	if(aslr_slide == 0) {
		// In order for us to calculate this offset ourself, we must have access
		//  to a dyldcache for this device and firmware version
		if(cache == NULL) {
			error("You must specify either --cache or --aslr-slide arguments\n");
			return -1;
		}

		// We have the required arguments, so we can calculate this parameter ourselves
		debug("Calculating dyldcache ASLR offset\n");
		aslr_slide = find_aslr_slide(crash, cache);
		if(aslr_slide == 0) {
			error("Unable to calculate ASLR offset\n");
		}
	}

	/*
	// TODO: Guess heap address
	// TODO: Craft attack string
	// TODO: Open Dyldcache
	// TODO: Find ROP offset
	// TODO: Open connection to mobilebackup
	mb2_t* mb2 = mb2_connect(device);
	if(mb2 == NULL) {
		error("Unable to open connection to mobilebackup\n");
		return -1;
	}
	*/
	// TODO: Register for mobilebackup download callback
	// TODO: Spray mobilebackup heap with function address
	// TODO: Send attack string to mobilebackup
	// TODO: If we crashed then grab the crashreport and repeat

	if(device) device_free(device);
	return 0;
}
