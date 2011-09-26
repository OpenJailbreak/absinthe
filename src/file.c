/**
  * GreenPois0n Apparition - file.c
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

#include "file.h"

int file_read(const char* file, unsigned char** buf, unsigned int* length) {
	FILE* fd = NULL;
	fd = fopen(file, "r+");
	if(fd == NULL) {
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	long size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	unsigned char* data = malloc(size);

	int bytes = fread(data, 1, size, fd);
	if(bytes != size) {
		fclose(fd);
		return -1;
	}
	fclose(fd);

	*buf = data;
	*length = bytes;
	return bytes;
}

int file_write(const char* file, unsigned char* buf, unsigned int length) {
	FILE* fd = NULL;
	fd = fopen(file, "w+");
	if(fd == NULL) {
		return -1;
	}

	int bytes = fwrite(buf, 1, length, fd);
	if(bytes != length) {
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return bytes;
}
