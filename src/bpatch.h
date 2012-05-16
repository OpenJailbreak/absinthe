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

#include <stdint.h>
#include <bzlib.h>

#include "boolean.h"

/*
	File format:
		0	8	"BSDIFF40" (bzip2) or "BSDIFN40" (raw)
		8	8	X
		16	8	Y
		24	8	sizeof(newfile)
		32	X	bzip2(control block)
		32+X	Y	bzip2(diff block)
		32+X+Y	???	bzip2(extra block)
	with control block a set of triples (x,y,z) meaning "add x bytes
	from oldfile to x bytes from the diff block; copy y bytes from the
	extra block; seek forwards in oldfile by z bytes".
	*/


typedef struct bpatch_header_t {
	uint8_t magic[8];
	int64_t ctrllen;
	int64_t datalen;
	int64_t filelen;
} bpatch_header_t;

typedef struct bpatch_t {
	char* path;
	uint8_t* data;
	uint8_t* extra;
	uint8_t* control;
	int64_t data_size;
	int64_t extra_size;
	int64_t control_size;
	int64_t data_offset;
	int64_t extra_offset;
	int64_t control_offset;
	bpatch_header_t* header;
} bpatch_t;

int bpatch(const char* in, const char* out, const char* patch);

bpatch_t* bpatch_create();
bpatch_t* bpatch_open(const char* path);
bpatch_t* bpatch_load(uint8_t* data, int64_t size);
int bpatch_apply(bpatch_t* bpatch, const char* path);
void bpatch_free(bpatch_t* bpatch);
void bpatch_debug(bpatch_t* bpatch);

/*
 * Binary Patch Header
 */
bpatch_header_t* bpatch_header_create();
bpatch_header_t* bpatch_header_load(uint8_t* data, int64_t size);
void bpatch_header_free(bpatch_header_t* header);
void bpatch_header_debug(bpatch_header_t* header);

unsigned int bpatch_decompress(char* input, unsigned int in_size, char* output, unsigned int* out_size);

#endif /* BPATCH_H_ */
