/**
 * GreenPois0n Absinthe - afc_utils.c
 * Copyright (C) 2012 Chronic-Dev Team
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

#ifndef WIN32
#include <libgen.h>
#endif

#include <libimobiledevice/afc.h>

#include "afc_utils.h"
#include "debug.h"

void afc_dictionary_free(char **dictionary)
{
	int i = 0;

	if (!dictionary)
		return;

	for (i = 0; dictionary[i]; i++) {
		free(dictionary[i]);
	}
	free(dictionary);
}

int afc_rmdir_recursive(afc_client_t afc, const char *path, int incl)
{
	char **dirlist = NULL;
	if (afc_read_directory(afc, path, &dirlist) != AFC_E_SUCCESS) {
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
		char *fpath = (char *) malloc(strlen(path) + 1 + strlen(*ptr) + 1);
		strcpy(fpath, path);
		strcat(fpath, "/");
		strcat(fpath, *ptr);
		if ((afc_get_file_info(afc, fpath, &info) != AFC_E_SUCCESS) || !info) {
			/* failed — try to delete anyway */
			afc_remove_path(afc, fpath);
			free(fpath);
			afc_dictionary_free(info);
			continue;
		}

		int is_dir = 0;
		int i;
		for (i = 0; info[i]; i += 2) {
			if (!strcmp(info[i], "st_ifmt")) {
				if (!strcmp(info[i + 1], "S_IFDIR")) {
					is_dir = 1;
				}
				break;
			}
		}
		afc_dictionary_free(info);

		if (is_dir) {
			afc_rmdir_recursive(afc, fpath, 0);
		}
		afc_remove_path(afc, fpath);
		free(fpath);
	}

	afc_dictionary_free(dirlist);
	if (incl) {
		afc_remove_path(afc, path);
	}

	return 0;
}

int afc_upload_file_as(afc_client_t afc, const char *filename,
		const char *dstfn)
{
	uint64_t handle = 0;
	char data[0x1000];

	FILE *infile = fopen(filename, "rb");
	if (!infile) {
		error("Unable to open local file %s\n", filename);
		return -1;
	}

	afc_error_t err = afc_file_open(afc, dstfn, AFC_FOPEN_WR, &handle);
	if (err != AFC_E_SUCCESS) {
		error("Unable to open afc://%s\n", dstfn);
		fclose(infile);
		return -1;
	}

	int res = 0;
	while (!feof(infile)) {
		uint32_t bytes_read = fread(data, 1, sizeof(data), infile);
		uint32_t bytes_written = 0;
		if (afc_file_write(afc, handle, data, bytes_read,
				&bytes_written) != AFC_E_SUCCESS) {
			error("Error writing to file afc://%s\n", dstfn);
			res = -1;
			break;
		}
	}
	afc_file_close(afc, handle);
	fclose(infile);

	return res;
}

int afc_upload_file(afc_client_t afc, const char *filename, const char *todir)
{
#ifdef WIN32
	int i = 0;
	int mfl = strlen(filename);
	char *bn = (char *) filename;
	for (i = mfl - 1; i >= 0; i--) {
		if ((bn[i] == '/') || (bn[i] == '\\')) {
			bn = &bn[i + 1];
			break;
		}
	}
#else
	char *bn = basename((char *) filename);
#endif
	char *dstfn = (char *) malloc(strlen(todir) + 1 + strlen(bn) + 1);
	strcpy(dstfn, todir);
	strcat(dstfn, "/");
	strcat(dstfn, bn);

	int res = afc_upload_file_as(afc, filename, dstfn);
	free(dstfn);
	return res;
}
