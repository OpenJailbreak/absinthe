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
#include "vpn_config.h"
#include <plist/plist.h>

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
			vpn_prefs_remove_entry(&pl);
			debug("Adding VPN connection entry\n");
			vpn_prefs_add_entry(&pl);

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
