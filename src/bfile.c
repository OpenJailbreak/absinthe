/**
  * GreenPois0n Absinthe - bfile.c
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

#include "bfile.h"
#include "common.h"

#define BUFSIZE 4096

bfile_t* bfile_create() {
	bfile_t* bfile = (bfile_t*) malloc(sizeof(bfile_t));
	if (bfile) {
		memset(bfile, '\0', sizeof(bfile_t));
	}
	return bfile;
}

bfile_t* bfile_open(const char* path) {
	uint64_t got = 0;
	uint8_t buffer[BUFSIZE+1];
	memset(buffer, '\0', BUFSIZE+1);

	bfile_t* bfile = bfile_create();
	if (bfile) {
		bfile->desc = fopen(path, "rb");
		if (bfile->desc == NULL) {
			fprintf(stderr, "Unable to open bfile %s\n", path);
			return NULL;
		}

		bfile->path = strdup(path);
		if (bfile->path == NULL) {
			fprintf(stderr, "Unable to allocate memory for bfile path\n");
			bfile_free(bfile);
			return NULL;
		}

		fseek(bfile->desc, 0, SEEK_END);
		bfile->size = ftell(bfile->desc);
		fseek(bfile->desc, 0, SEEK_SET);

		bfile->offset = 0;
		bfile->data = (unsigned char*) malloc(bfile->size+1);
		if (bfile->data == NULL) {
			fprintf(stderr, "Unable to allocate memory for bfile\n");
			bfile_free(bfile);
			return NULL;
		}

		uint64_t offset = 0;
		while (offset < bfile->size) {
			memset(buffer, '\0', BUFSIZE);
			got = fread(buffer, 1, BUFSIZE, bfile->desc);
			if (got > 0) {
				memcpy(&bfile->data[offset], buffer, got);
				offset += got;
			} else {
				break;
			}
		}
		fprintf(stderr, "Read in %llu of %llu bytes from %s\n", got, bfile->size, bfile->path);
		// We have the data stored in memory now, so we don't need to keep this open anymore
		//fseek(bfile->desc, 0, SEEK_SET);
		bfile_close(bfile);
		bfile->offset = 0;
	}
	return bfile;
}

void bfile_close(bfile_t* bfile) {
	if (bfile) {
		if (bfile->desc) {
			fclose(bfile->desc);
			bfile->desc = NULL;
		}
	}
}

void bfile_free(bfile_t* bfile) {
	if (bfile) {
		if (bfile->desc) {
			bfile_close(bfile);
			bfile->desc = NULL;
		}
		if (bfile->path) {
			free(bfile->path);
			bfile->path = NULL;
		}
		if(bfile->data) {
			free(bfile->data);
			bfile->data = NULL;
		}
		bfile->size = 0;
		bfile->offset = 0;
		free(bfile);
	}
}

unsigned int bfile_read(bfile_t* bfile, unsigned char* data, unsigned int size) {
	int len = 0;
	if(bfile->offset + size > bfile->size) {
		len = bfile->size - bfile->offset;
	} else {
		len = size;
	}

	memcpy(data, &bfile->data[bfile->offset], len);
	bfile->offset += len;
	return len;
}

unsigned int bfile_write(bfile_t* bfile, unsigned char* data, unsigned int size) {
	int len = 0;
	if(bfile->offset + size > bfile->size) {
		len = bfile->size - bfile->offset;
	} else {
		len = size;
	}

	memcpy(&bfile->data[bfile->offset], data, len);
	bfile->offset += len;
	return len;
}

long bfile_tell(bfile_t* bfile) {
	return bfile->offset;
}

void bfile_seek(bfile_t* bfile, long offset) {
	bfile->offset = offset;
}
