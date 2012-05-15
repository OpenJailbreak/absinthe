/**
  * GreenPois0n Apparition - bpatch.h
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

#ifndef BPATCH_H_
#define BPATCH_H_

#include <bzlib.h>

#include "bfile.h"

typedef struct bpatch_t {
	char* path;
	bfile_t* file;
	bz_stream* stream;
	unsigned int size;
	unsigned int offset;
	unsigned char* data;
	unsigned char* input;
	unsigned char* output;
} bpatch_t;

bpatch_t* bpatch_open(const char* path);
int bpatch_apply(bpatch_t* patch, const char* path);
void bpatch_free(bpatch_t* bpatch);

int bpatch(const char* in, const char* out, const char* patch);
unsigned int bpatch_read(bpatch_t* stream, int *bzerr, unsigned char* out, unsigned int len);


#endif /* BPATCH_H_ */
