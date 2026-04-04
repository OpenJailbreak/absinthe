/**
 * GreenPois0n Absinthe - afc_utils.h
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

#ifndef AFC_UTILS_H
#define AFC_UTILS_H

#include <libimobiledevice/afc.h>

/* Free a NULL-terminated array of strings (as returned by AFC directory APIs). */
void afc_dictionary_free(char **dictionary);

/* Recursively remove a path via AFC.
 * If incl is non-zero, the path itself is removed; otherwise only its contents. */
int afc_rmdir_recursive(afc_client_t afc, const char *path, int incl);

/* Upload a local file to a specific destination path on the device via AFC. */
int afc_upload_file_as(afc_client_t afc, const char *filename, const char *dstfn);

/* Upload a local file to a destination directory on the device via AFC.
 * The basename of filename is appended to todir. */
int afc_upload_file(afc_client_t afc, const char *filename, const char *todir);

#endif /* AFC_UTILS_H */
