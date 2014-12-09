/**
 * GreenPois0n Absinthe - common.c
 * Copyright (C) 2011 Chronic-Dev Team
 * Copyright (C) 2011 Nikias Bassen
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

#include <dirent.h>

#include "debug.h"
#include "common.h"
#include "boolean.h"

int __mkdir(const char* path, int mode)
{
#ifdef WIN32
	return mkdir(path);
#else
	return mkdir(path, mode);
#endif
}

int mkdir_with_parents(const char *dir, int mode)
{
	if (!dir) return -1;
	if (__mkdir(dir, mode) == 0) {
		return 0;
	} else {
		if (errno == EEXIST) return 0;	
	}
	int res;
	char *parent = strdup(dir);
	parent = dirname(parent);
	if (parent) {
		res = mkdir_with_parents(parent, mode);
	} else {
		res = -1;	
	}
	free(parent);
	if (res == 0) {
		mkdir_with_parents(dir, mode);
	}
	return res;
}

char* build_path(const char* elem, ...)
{
	if (!elem) return NULL;
	va_list args;
	int len = strlen(elem)+1;
	va_start(args, elem);
	char *arg = va_arg(args, char*);
	while (arg) {
		len += strlen(arg)+1;
		arg = va_arg(args, char*);
	}
	va_end(args);

	char* out = (char*)malloc(len);
	strcpy(out, elem);

	va_start(args, elem);
	arg = va_arg(args, char*);
	while (arg) {
		strcat(out, "/");
		strcat(out, arg);
		arg = va_arg(args, char*);
	}
	va_end(args);
	return out;
}

/*
 * This function converts the unix permission bits into understandable letters. OMG!
 */
char* prot2str(uint32_t prot) {
	int i = 0;
	char* str = (char*) malloc(BUFSMALL);
	if(str) {
		memset(str, '\0', BUFSMALL);
		if(prot & 4) {
			str[i++] = 'w';
		} else str[i++] = '-';
		if(prot & 2) {
			str[i++] = 'r';
		} else str[i++] = '-';
		if(prot & 1) {
			str[i++] = 'x';
		} else str[i++] = '-';
	}
	return str;
}

int check_ascii_string(const char* string, size_t length) {
	size_t i = 0;
	if (string) {
		// Loop through each byte in this string and make sure it contains no invalid
		//  ASCII characters that might screw up our exploit
		for (i = 0; i < length; i++) {
			char letter = string[i];
			if ((letter & 0x80) > 0 || (letter & 0x7F) == 0) {
				// We have an invalid ASCII character here folks!
				return kFalse;
			}
		}
	}

	return kTrue;
}

int check_ascii_pointer(uint32_t pointer) {
	if((pointer & 0x80808080) > 0) {
		//debug("FAIL\n");
		return 0;
	}
	//debug("Passed ASCII test\n");
	if((pointer & 0x7F000000) == 0 ||
		(pointer & 0x007F0000) == 0 ||
		(pointer & 0x00007F00) == 0 ||
		(pointer & 0x0000007F) == 0) {
		//debug("FAIL\n");
		//debug("0x%08x & 0x7F7F7F7F = 0x%08x\n", pointer, (pointer & 0x7F7F7F7F));
		return 0;
	}
	//debug("PASS\n");
	return 1;
}

void hexdump(unsigned char* buf, unsigned int len) {
	int i, j;
	debug("0x%08x: ", buf);
	for (i = 0; i < len; i++) {
		if (i % 16 == 0 && i != 0) {
			for (j=i-16; j < i; j++) {
				unsigned char car = buf[j];
				if (car < 0x20 || car > 0x7f) car = '.';
				debug("%c", car);
			}
			debug("\r\n0x%08x: ", buf+i);
		}
		debug("%02x ", buf[i]);
	}

	int done = (i % 16);
	int remains = 16 - done;
	if (done > 0) {
		for (j = 0; j < remains; j++) {
			debug("   ");
		}
	}

	if ((i - done) >= 0) {
		if (done == 0 && i > 0) done = 16;
		for (j = (i - done); j < i; j++) {
			unsigned char car = buf[j];
			if (car < 0x20 || car > 0x7f) car = '.';
			debug("%c", car);
		}
	}

	debug("\r\n\n");
}

/* recursively remove path, including path */
void rmdir_recursive(const char *path) /*{{{*/
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
						debug("could not remove file %s: %s\n", fpath, strerror(errno));
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
