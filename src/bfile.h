/**
  * GreenPois0n Absinthe - bfile.h
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

#ifndef bfile_H_
#define bfile_H_

#include <stdint.h>

typedef struct bfile_t {
	FILE* desc;
	char* path;
	uint64_t size;
	uint64_t offset;
	unsigned char* data;
} bfile_t;

bfile_t* bfile_create();
bfile_t* bfile_open(const char* path);

void bfile_close(bfile_t* bfile);
void bfile_free(bfile_t* bfile);

unsigned int bfile_read(bfile_t* bfile, unsigned char* data, unsigned int size);
unsigned int bfile_write(bfile_t* bfile, unsigned char* data, unsigned int size);

long bfile_tell(bfile_t* bfile);
void bfile_seek(bfile_t* bfile, long offset);

#endif
