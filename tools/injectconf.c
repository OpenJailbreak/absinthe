/*
 * injectconf.c
 *
 *  Created on: Feb 28, 2013
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "device.h"
#include "common.h"
#include "backup.h"
#include "backup_file.h"
#include "plist_extras.h"
#include "mbdb.h"
#include "mbdb_record.h"
#include "jailbreak.h"
#include <plist/plist.h>

static int get_rand(int min, int max) /*{{{*/
{
	int retval = (rand() % (max - min)) + min;
	return retval;
} /*}}}*/

static char *generate_guid() /*{{{*/
{
	char *guid = (char *) malloc(sizeof(char) * 37);
	const char *chars = "ABCDEF0123456789";
	srand(time(NULL ));
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
							printf(
									"removing /NetworkServices/%s (UserDefinedName: %s)\n",
									key, uname_str);
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
				plist_t nn = plist_access_path(n, 3, "Network", "Service",
						guid);
				if (nn) {
					nn = plist_access_path(n, 2, "Network", "Service");
					if (nn) {
						fprintf(stderr,
								"removing /Sets/%s/Network/Service/%s\n", key,
								guid);
						plist_dict_remove_item(nn, guid);
					}
				}
				nn = plist_access_path(n, 4, "Network", "Global", "IPv4",
						"ServiceOrder");
				if (nn && (plist_get_node_type(nn) == PLIST_ARRAY)) {
					int32_t num = (int32_t) plist_array_get_size(nn);
					int32_t x;
					for (x = num - 1; x >= 0; x--) {
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
								fprintf(stderr,
										"removing /Sets/%s/Network/Global/IPv4/ServiceOrder/%s\n",
										key, guid);
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
	plist_dict_insert_item(conn, "UserDefinedName",
			plist_new_string(CONNECTION_NAME));

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
	plist_dict_insert_item(dict, "AuthenticationMethod",
			plist_new_string("SharedSecret"));
	plist_dict_insert_item(dict, "OnDemandSupported", plist_new_bool(1));
	plist_dict_insert_item(dict, "OnDemandEnabled", plist_new_uint(1));
	arr = plist_new_array();
	plist_array_append_item(arr, plist_new_string(VPN_TRIGGER_ADDRESS));
	plist_dict_insert_item(dict, "OnDemandMatchDomainsAlways", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsNever", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsOnRetry", arr);
	plist_dict_insert_item(dict, "RemoteAddress",
			plist_new_string("127.0.0.1"));
	plist_dict_insert_item(dict, "LocalIdentifier", plist_new_string("2"));
	plist_dict_insert_item(dict, "XAuthName",
			plist_new_string(
					"\";mode_cfg on;proposal { authentication_method xauth_psk_client; hash_algorithm sha1; encryption_algorithm aes 256; lifetime time 3600 sec; dh_group 2; } include \"/private/var/preferences/SystemConfiguration/com.apple.ipsec.plist\"; proposal {"));
	plist_dict_insert_item(dict, "LocalIdentifierType",
			plist_new_string("KeyID"));
	plist_dict_insert_item(dict, "XAuthEnabled", plist_new_uint(1));
	plist_dict_insert_item(dict, "SharedSecretEncryption",
			plist_new_string("Key"));
	plist_dict_insert_item(dict, "SharedSecret", plist_new_string("ninja"));
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
		fprintf(stderr, "ERROR: CurrentSet has unexpected string value '%s'\n",
				(curset) ? "(null)" : curset);
		if (curset) {
			free(curset);
		}
		return;
	}

	// locate /Sets/{SetGUID}/Network/Service node
	plist_t netsvc = plist_access_path(*pl, 4, "Sets", curset + 6, "Network",
			"Service");
	if (!netsvc) {
		fprintf(stderr,
				"ERROR: Could not access /Sets/%s/Network/Service node\n",
				curset + 6);
		free(curset);
		return;
	}

	// locate /Sets/{SetGUID}/Network/Global/IPv4/ServiceOrder node
	plist_t order = plist_access_path(*pl, 6, "Sets", curset + 6, "Network",
			"Global", "IPv4", "ServiceOrder");
	if (!order) {
		fprintf(stderr,
				"ERROR: Could not access /Sets/%s/Network/Global/IPv4/ServiceOrder node\n",
				curset + 6);
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
	char *linkstr = malloc(17 + strlen(guid) + 1);
	strcpy(linkstr, "/NetworkServices/");
	strcat(linkstr, guid);
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "__LINK__", plist_new_string(linkstr));
	free(linkstr);
	plist_dict_insert_item(netsvc, guid, dict);

	// done
	free(guid);
} /*}}}*/

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("usage: injectconf <backupdir> <conf>\n");
		return 0;
	}

	device_t* device = device_create(NULL );
	char* udid = device->udid;

	char* backup_directory = argv[1];
	device_free(device);

	backup_t* backup = backup_open(backup_directory, udid);
	if (!backup) {
		fprintf(stderr, "ERROR: failed to open backup\n");
		return -1;
	}

	/********************************************************/
	/* add vpn on-demand connection to preferences.plist */
	/********************************************************/
	printf("Preparing jailbreak files...\n");

	backup_file_t* bf = NULL;
	bf = backup_get_file(backup, "SystemPreferencesDomain",
			"SystemConfiguration/preferences.plist");
	if (bf) {
		char* fn = backup_get_file_path(backup, bf);
		if (!fn) {
			fprintf(stderr,
					"Huh, backup path for preferences.plist not found?!\n");
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

			plist_to_bin(pl, (char**) &prefs, &plen);

			backup_file_assign_file_data(bf, prefs, plen, 1);
			free(prefs);
			prefs = NULL;
			backup_file_set_length(bf, plen);
			backup_file_update_hash(bf);

			if (backup_update_file(backup, bf) < 0) {
				fprintf(stderr, "ERROR: could not add file to backup\n");
			} else {
				backup_write_mbdb(backup);
				printf("Done!\n");
			}

		} else {
			fprintf(stderr, "Could not open '%s'\n", fn);
		}
		free(fn);
	} else {
		fprintf(stderr,
				"ERROR: could not locate preferences.plist in backup.\n");
	}
	backup_file_free(bf);
	//backup_free(backup);

	//backup = backup_open(backup_directory, udid);
	if (!backup) {
		error("ERROR: failed to open backup\n");
		return -1;
	}
	char* ipsec_plist = NULL;
	int ipsec_plist_size = 0;
	file_read(argv[2], (unsigned char**) &ipsec_plist, &ipsec_plist_size);
	bf = backup_get_file(backup, "SystemPreferencesDomain",
			"SystemConfiguration/com.apple.ipsec.plist");
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
		unsigned int tm = (unsigned int) (time(NULL ));
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
	return 0;
}
