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
#include "common.h"

int mkdir_with_parents(const char *dir, int mode)
{
	if (!dir) return -1;
	if (mkdir(dir, mode) == 0) {
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
	printf("elem: %s\n", elem);
	va_start(args, elem);
	char *arg = va_arg(args, char*);
	while (arg) {
		len += strlen(arg)+1;
		printf("elem: %s\n", arg);
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

	printf("build_path: %s\n", out);

	return out;
}

