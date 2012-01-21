/**
 * GreenPois0n Absinthe - crashreporter.c
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
#include <stddef.h>

#include "debug.h"
#include "file.h"
#include "lockdown.h"
#include "crashreport.h"
#include "crashreporter.h"
#include "crashreportcopy.h"
#include "crashreportmover.h"

crashreporter_t* crashreporter_create() {
	crashreporter_t* crashreporter = (crashreporter_t*) malloc(sizeof(crashreporter_t));
	if(crashreporter) {
		memset(crashreporter, '\0', sizeof(crashreporter_t));
	}
	return crashreporter;
}

crashreporter_t* crashreporter_connect(device_t* device) {
	int err = 0;
	uint16_t port = 0;
	crashreporter_t* crashreporter = crashreporter_create();
	if(crashreporter != NULL) {
		crashreporter->mover = crashreportmover_connect(device);
		if(crashreporter->mover == NULL) {
			error("Unable to connect to CrashReporter's mover service\n");
			return NULL;
		}

		crashreporter->copier = crashreportcopy_connect(device);
		if(crashreporter->copier == NULL) {
			error("Unable to connect to CrashReporter's copier service\n");
			return NULL;
		}
	}

	return crashreporter;
}

crashreporter_t* crashreporter_open(device_t* device, uint16_t port) {
	crashreporter_t* crashreporter = crashreporter_create();
	if(crashreporter == NULL) {
		error("Unable to create CrashReporter client\n");
		return NULL;
	}
	// Startup crashreportmover service to move our crashes to the proper place ???
	crashreportmover_t* mover = crashreportmover_open(device, port);
	if(mover == NULL) {
		
		printf("failed to open crashreportermover_open!\n");
		
		return NULL;
	}
	
	// Startup crashreporter copy to copy them to mobile root??	
	crashreportcopy_t* copier = crashreportcopy_open(device, port);
	if(copier == NULL) {
		//crashreportmover_free(mover);
		return NULL;
	}
	
	crashreporter->mover = mover;
	crashreporter->copier = copier;
	crashreporter->device = device;

	return crashreporter;
}

int crashreporter_close(crashreporter_t* crashreporter) {
	return -1;
}

void crashreporter_free(crashreporter_t* crashreporter) {
	if (crashreporter) {
		if (crashreporter->mover) {
			crashreportmover_free(crashreporter->mover);
		}
		if (crashreporter->copier) {
			crashreportcopy_free(crashreporter->copier);
		}
		free(crashreporter);
	}
}


crashreport_t* crashreporter_last_crash(crashreporter_t* crashreporter) {
	char** list = NULL;
	afc_error_t err = AFC_E_SUCCESS;
	if(crashreporter == NULL) {
		return NULL;
	}

	if(crashreporter->mover == NULL) {
		return NULL;
	}

	if(crashreporter->copier == NULL) {
		return NULL;
	}

	err = afc_read_directory(crashreporter->copier->client, "/", &list);
	if(err != AFC_E_SUCCESS) {
		return NULL;
	}

	char *lastItem = NULL;

	int i = 0;
	int j = 0;

	time_t latest = 0;

	for(i = 0; list[i] != NULL; i++) {
		if (!(strstr(list[i], "BackupAgent_") && strstr(list[i], ".plist"))) continue;

		char **info = NULL;
		if (afc_get_file_info(crashreporter->copier->client, list[i], &info) != AFC_E_SUCCESS) continue;
		if (!info) continue;
		time_t mtime = 0;
		for (j = 0; info[j]; j += 2) {
			if (!strcmp(info[j], "st_mtime")) {
				mtime = atoll(info[j+1])/1000000000;
			}
			free(info[j]);
			free(info[j+1]);
		}
		free(info);
		if (mtime >= latest) {
			latest = mtime;
			lastItem = list[i];
		}
	}

	printf("Copying '%s'\n", lastItem);
	if (lastItem) {
		lastItem = strdup(lastItem);
	}
	for (i = 0; list[i]; i++) {
		free(list[i]);
	}
	free(list);
	if (!lastItem) {
		printf("hmm.. could not get last item\n");
		return NULL;
	}

	uint64_t handle;
	char data[0x1000];

	err = afc_file_open(crashreporter->copier->client, lastItem, AFC_FOPEN_RDONLY, &handle);
	if(err != AFC_E_SUCCESS) {
		printf("Unable to open %s\n", lastItem);
		free(lastItem);
		return NULL;
	}

        char crash_file[1024];
        tmpnam(crash_file);

	FILE* output = fopen(crash_file, "wb");
	if(output == NULL) {
		printf("Unable to open local file %s\n", crash_file);
		free(lastItem);
		afc_file_close(crashreporter->copier->client, handle);
		return NULL;
	}

	int bytes_read = 0;
	err = afc_file_read(crashreporter->copier->client, handle, data, 0x1000, &bytes_read);
	while(err == AFC_E_SUCCESS && bytes_read > 0) {
		fwrite(data, 1, bytes_read, output);
		err = afc_file_read(crashreporter->copier->client, handle, data, 0x1000, &bytes_read);
	}
	afc_file_close(crashreporter->copier->client, handle);
	fclose(output);

	afc_remove_path(crashreporter->copier->client, lastItem);

	uint32_t size = 0;
	plist_t plist = NULL;
	int ferr = 0;
	unsigned char* datas = NULL;
	ferr = file_read(crash_file, &datas, &size);
	if (ferr < 0) {
		fprintf(stderr, "Unable to open %s\n", crash_file);
		free(lastItem);
		return NULL;
	}

	plist_from_xml(datas, size, &plist);
	free(datas);

	crashreport_t* report = NULL;
	if (plist) {
		report = crashreport_parse_plist(plist);
		plist_free(plist);
		remove(crash_file);
	} else {
		error("Reading crash report as plist failed\n");
	}
	free(lastItem);
	return report;
}
