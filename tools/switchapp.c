/*
 * appswitch.c
 *
 *  Created on: Jan 20, 2013
 *      Author: posixninja
 */

#include <stdio.h>

#include "debug.h"
#include "device.h"
#include "common.h"
#include "backup.h"
#include "backup_file.h"

#define AFCTMP "HackStore"

int main(int argc, char* argv[]) {
	debug("Preparing to open device\n");
	device_t* device = device_create(NULL );
	if (device == NULL ) {
		error("Unable to open device\n");
		return -1;
	}
	debug("Opened device %s\n", device->udid);
	//device_enable_debug();

	debug("Preparing to create device backup\n");
	char *bargv[] = { "idevicebackup2", "backup", ".", NULL };
	idevicebackup2(3, bargv);
	debug("Create device backup\n");

	backup_t* backup = backup_open(".", device->udid);
	if (!backup) {
		error("Unable to open backup\n");
		device_free(device);
		return -1;
	}

	backup_

	/*
	unsigned int tm = (unsigned int) (time(NULL ));
	debug("Adding new file to device\n");
	backup_file_t* bf = backup_file_create(NULL);
	if (bf) {
		backup_file_set_domain(bf, "BooksDomain");
		backup_file_set_path(bf, "blah");
		backup_file_set_target(bf, "blah");
		backup_file_set_mode(bf, 0120644);
		backup_file_set_inode(bf, 54327);
		backup_file_set_uid(bf, 0);
		backup_file_set_gid(bf, 0);
		backup_file_set_time1(bf, tm);
		backup_file_set_time2(bf, tm);
		backup_file_set_time3(bf, tm);
		backup_file_set_flag(bf, 0);

		if (backup_update_file(backup, bf) < 0) {
			fprintf(stderr, "ERROR: could not add file to backup\n");
		}
		backup_file_free(bf);
	}

	backup_write_mbdb(backup);
	backup_free(backup);
	*/

	debug("Preparing to restore device backup\n");
	char* rargv[] = { "idevicebackup2", "restore", "--system", "--settings",
			".", NULL };
	idevicebackup2(6, rargv);
	debug("Restored device backup\n");

	debug("Closing device\n");
	device_free(device);
	return 0;
}
