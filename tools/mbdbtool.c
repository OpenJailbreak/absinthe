/*
 * mbdbtool.c
 *
 *  Created on: Mar 22, 2013
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "backup.h"

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("usage: mbdbtool <dir> <domain> <cmd> [args]\n");
		return 0;
	}

	device_t* device = device_create(NULL );
	if (device == NULL ) {
		printf("Unable to find device, make sure it's connected\n");
		return 0;
	}
	char* udid = strdup(device->udid);
	device_free(device);
	printf("Found device with UUID %s\n", udid);

	char* dir = strdup(argv[1]);
	char* dom = strdup(argv[2]);
	char* cmd = strdup(argv[3]);

	if (strcmp(cmd, "ls") == 0) {
		if (argc != 4) {
			printf("usage: mbdbtool <dir> <domain> ls <path>\n");
			free(udid);
			free(cmd);
			free(dir);
			free(dom);
			return 0;
		}
		backup_t* backup = backup_open(dir, udid);
		// List the directory
		backup_free(backup);
	} else if (strcmp(cmd, "get") == 0) {
		if (argc != 6) {
			printf("usage: mbdbtool <dir> <domain> get <remote> <local>\n");
			free(udid);
			free(cmd);
			free(dir);
			free(dom);
			return 0;
		}
		backup_t* backup = backup_open(dir, udid);
		if (backup) {
			printf("Backup opened\n");
			backup_file_t* file = backup_get_file(backup, dom, argv[4]);
			if (file) {
				char* path = backup_get_file_path(backup, file);
				printf("Got file %s\n", path);
				unsigned int size = 0;
				unsigned char* data = NULL;
				file_read(path, &data, &size);
				file_write(argv[5], data, size);
				free(data);
				backup_file_free(file);
			}
			backup_free(backup);
		}
	} else if (strcmp(cmd, "put") == 0) {
		if (argc != 6) {
			printf("usage: mbdbtool <dir> <domain> put <local> <remote>\n");
			free(udid);
			free(cmd);
			free(dir);
			free(dom);
			return 0;
		}
		backup_t* backup = backup_open(dir, udid);
		if (backup) {
			printf("Backup opened\n");
			unsigned int size = 0;
			unsigned char* data = NULL;
			file_read(argv[4], &data, &size);
			backup_file_t* file = backup_get_file(backup, dom, argv[5]);
			if (file) {
				printf("Found file, replacing it\n");
				char* fn = backup_get_file_path(backup, file);
				if (fn != NULL ) {
					file_write(fn, &data, &size);
					backup_file_assign_file_data(file, data, size, 1);
					backup_file_set_length(file, size);
					backup_file_update_hash(file);
					backup_update_file(backup, file);
					backup_write_mbdb(backup);
					backup_file_free(file);
				} else {
					printf("Error, unable to find file in backup\n");
				}
			} else {
				printf("File not found, creating new file\n");

				unsigned int tm = (unsigned int) (time(NULL ));
				file = backup_file_create_with_data(data, size, 0);
				backup_file_set_domain(file, dom);
				backup_file_set_path(file, argv[5]);
				backup_file_set_mode(file, 0100644);
				backup_file_set_inode(file, 123456);
				backup_file_set_uid(file, 0);
				backup_file_set_gid(file, 0);
				backup_file_set_time1(file, tm);
				backup_file_set_time2(file, tm);
				backup_file_set_time3(file, tm);
				backup_file_set_length(file, size);
				backup_file_set_flag(file, 4);
				backup_file_update_hash(file);
				backup_update_file(backup, file);
				backup_write_mbdb(backup);
				backup_file_free(file);
			}
			backup_free(backup);
		}
	} else {
		printf("Unknown command %s\n", cmd);
	}

	free(udid);
	free(cmd);
	free(dir);
	free(dom);
	return 0;
}
