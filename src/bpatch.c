/**
 * GreenPois0n Apparition - bpatch.c
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
#include <stdint.h>

#include <bzlib.h>

#include "bpatch.h"
#include "debug.h"
#include "common.h"
#include "boolean.h"

#define BUFSIZE 512000

static uint64_t offtin(uint8_t* buf) {
	uint64_t y;

	y = buf[7] & 0x7F;
	y <<= 8;
	y += buf[6];
	y <<= 8;
	y += buf[5];
	y <<= 8;
	y += buf[4];
	y <<= 8;
	y += buf[3];
	y <<= 8;
	y += buf[2];
	y <<= 8;
	y += buf[1];
	y <<= 8;
	y += buf[0];

	if (buf[7] & 0x80)
		y = -y;

	return y;
}

bpatch_t* bpatch_create() {
	bpatch_t* bpatch = (bpatch_t*) malloc(sizeof(bpatch_t));
	if (bpatch) {
		memset(bpatch, '\0', sizeof(bpatch_t));
	}
	return bpatch;
}

bpatch_t* bpatch_open(const char* path) {
	uint32_t size = 0;
	uint8_t* data = NULL;
	file_read(path, &data, &size);
	if (data == NULL || size <= 0) {
		error("Unable to open binary patch file\n");
		return NULL;
	}

	bpatch_t* bpatch = bpatch_load(data, size);
	if (bpatch == NULL) {
		error("Unable to load binary patch file\n");
		return NULL;
	}

	bpatch->path = strdup(path);
	if (bpatch->path == NULL) {
		bpatch_free(bpatch);
		return NULL;
	}

	return bpatch;
}

void bpatch_free(bpatch_t* bpatch) {
	if (bpatch) {
		if (bpatch->path) {
			free(bpatch->path);
			bpatch->path = NULL;
		}
		free(bpatch);
	}
}


bpatch_t* bpatch_load(uint8_t* data, uint64_t size) {
	int err = 0;
	uint8_t* buf = NULL;
	int64_t offset = 0;
	uint64_t buf_size = 0;
	uint64_t data_size = 0;
	uint64_t extra_size = 0;
	uint64_t control_size = 0;

	bpatch_t* bpatch = bpatch_create();
	if (bpatch != NULL) {
		bpatch->header = bpatch_header_load(&data[offset], sizeof(bpatch_header_t));
		if (bpatch->header == NULL) {
			error("Unable to load binary patch header\n");
			bpatch_free(bpatch);
			return NULL;
		}
		offset += sizeof(bpatch_header_t);

		buf = malloc(BUFSIZE + 1);
		if(buf == NULL) {
			error("Unable to allocate buffer\n");
			return NULL;
		}

		//////////////////////////
		// Load in control block
		buf_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(bpatch, &data[offset], bpatch->header->ctrllen, buf, &buf_size);
		if (err < 0 || buf_size <= 0) {
			error("Unable to decompress control block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		// Make sure there's enough data left
		if(bpatch->header->ctrllen + offset > size) {
		  	error("Sanity check failed\n");
		  	bpatch_free(bpatch);
		  	return NULL;
		}

		// Allocate memory for our decompressed control block
		bpatch->control_size = buf_size;
		bpatch->control = (uint8_t*) malloc(buf_size + 1);
		if(bpatch->control == NULL) {
			error("Unable to allocate control block\n");
			bpatch_free(bpatch);
			return NULL;
		}
		memset(bpatch->control, '\0', buf_size);
		memcpy(bpatch->control, buf, buf_size);
		offset += bpatch->header->ctrllen;

		//////////////////////////
		// Load in diff block
		buf_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(bpatch, &data[offset], bpatch->header->datalen, buf, &buf_size);
		if (err < 0 || buf_size <= 0) {
			error("Unable to decompress diff block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		// Allocate memory for our diff block
		bpatch->data_size = data_size;
		bpatch->data = malloc(data_size + 1);
		if(bpatch->data == NULL) {
			error("Unable to allocate memory for diff block\n");
			return NULL;
		}
		memset(bpatch->data, '\0', data_size);
		memcpy(bpatch->data, buf, data_size);
		offset += bpatch->header->datalen;

		////////////////////////
		// Load in extra block
		buf_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(bpatch, &data[offset], (size - offset), buf, &buf_size);
		if (err < 0 || buf_size <= 0) {
			error("Unable to decompress extra block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		bpatch->extra_size = buf_size;
		bpatch->extra = malloc(bpatch->extra_size);
		if(bpatch->extra == NULL) {
			error("Unable to allocate memory for extra block\n");
			bpatch_free(bpatch);
			return NULL;
		}
		memset(bpatch->extra, '\0', BUFSIZE);
	}

	return bpatch;
}

void bpatch_debug(bpatch_t* bpatch) {
	if (bpatch) {
		debug("Binary Patch\n");
		if (bpatch->header) {
			bpatch_header_debug(bpatch->header);
		}
		if (bpatch->path != NULL) {
			debug("path = %s\n", bpatch->path);
		}
		if (bpatch->control_size > 0 && bpatch->control_size < BUFSIZE) {
			debug("Control:\n"); debug("\tsize = %llu\n", bpatch->control_size);
			if (bpatch->control != NULL) {
				//debug("compressed: \n");
				//hexdump(&bpatch->data[sizeof(bpatch_header_t)], bpatch->header->ctrllen);
				//debug("decompressed: \n");
				//hexdump(bpatch->control, bpatch->control_size);
			}
		}
		if (bpatch->data_size > 0 && bpatch->data_size < BUFSIZE) {
			debug("Data:\n"); debug("\tsize = %llu\n", bpatch->data_size);
			if (bpatch->data != NULL  && bpatch->data_size <= 512) {
				//debug("compressed: \n");
				//hexdump(&bpatch->data[sizeof(bpatch_header_t)], bpatch->header->ctrllen);
				//debug("decompressed: \n");
				//hexdump(bpatch->data, bpatch->data_size);
			}
		}
		// hexdump(bpatch->data, bpatch->data_size);

	}
}

int bpatch_apply(bpatch_t* bpatch, const char* path) {
	int i = 0;
	int64_t x = 0;
	int64_t y = 0;
	int64_t z = 0;
	int64_t ctrl[3];

	int64_t ctrl_size = 0;
	int64_t source_size = 0;
	int64_t target_size = 0;

	int64_t ctrl_offset = 0;
	int64_t data_offset = 0;
	int64_t extra_offset = 0;
	int64_t source_offset = 0;
	int64_t target_offset = 0;

	int8_t* source_data = NULL;
	int8_t* target_data = NULL;
	int8_t* ctrl_data = NULL;
	int8_t* cur_data = NULL;

	ctrl_data = bpatch->control;
	ctrl_size = bpatch->control_size;
	target_size = bpatch->header->filelen;

	if (path != NULL) {
		file_read(path, &source_data, &source_size);
		if (source_data == NULL || source_size <= 0) {
			error("Unable to read target file %s\n", path);
			return -1;
		}

		target_data = (uint8_t*) malloc(target_size + 1);
		if(target_data == NULL) {
			error("Unable to allocate data for new file\n");
			return -1;
		}
		memset(target_data, '\0', target_size);

		int64_t ctrl_start = *((int64_t*)&ctrl_data);
		int64_t ctrl_end = ctrl_start + ctrl_size;
		int64_t ctrl_position = ctrl_start + ctrl_offset;
		while (ctrl_position < ctrl_end) {
			// Loop 3 times to read in X, Y, and Z values from the control vector
			for (i = 0; i <= 2; i++) {
				ctrl[i] = offtin(bpatch->control + ctrl_offset);
				ctrl_offset += 8;
			}
			x = ctrl[0]; y = ctrl[1]; z = ctrl[2];
			debug("x = %qd, y = %qd, z = %qd\n", x, y, z);


			for(i = 0; i <= x; i++) {
				if(source_data + i >= 0 && source_data[source_offset+i] < source_size) {
					uint8_t value1 = source_data[source_offset+i];
					uint8_t value2 = bpatch->data[data_offset+i];
					target_data[target_offset+i] = value1 + value2;
				}
			}
			hexdump(&target_data[target_offset], 0x200);

			memcpy(&target_data[target_offset], &source_data[source_offset], x);
			debug("0x%qx:\tCopying %qd bytes from old file to new file\n", target_offset, x);
			hexdump(&target_data[target_offset], 0x200);
			target_offset += x;
			source_offset += x;
			//data_offset += x;


			//hexdump(&target_data[new_offset], 0x200);
			//for(i = 0; i < y; i++) {
			//	new_data[new_offset+i] = bpatch->extra[extra_offset+i];
			//}
			//hexdump(&target_data[new_offset], 0x200);

				//memcpy(&target_data[target_offset], bpatch->extra, y);
				//debug("0x%qx:\tCopying %qd bytes from extra block into new file\n", target_offset, y);
				//target_offset += y;
				//data_offset += y;
				//extra_offset += y;
				//hexdump()

			if(z != 0) {
				debug("0x%qx:\tSeeking to offset 0x%qx in the old file\n", target_offset, source_offset+z);
				hexdump(&target_data[target_offset], 0x200);
				source_offset += z;
			}

			// Prime loop for the next control vector thingy
			ctrl_position = ctrl_start + ctrl_offset;
		}

		// CleanUp
		free(source_data);
		file_write("racoon.pwn", target_data, target_size);
	}
	return 0;
}

/*
 * Binary Patch Header
 */
bpatch_header_t* bpatch_header_create() {
	bpatch_header_t* header = (bpatch_header_t*) malloc(
			sizeof(bpatch_header_t));
	if (header) {
		memset(header, '\0', sizeof(bpatch_header_t));
	}
	return header;
}

bpatch_header_t* bpatch_header_load(uint8_t* data, uint64_t size) {
	bpatch_header_t* header = bpatch_header_create();
	if (header != NULL) {
		if (memcmp(data, "BSDIFF40", 8) != 0) {
			error("Unable to find magic string in binary patch header");
			bpatch_header_free(header);
			return NULL;
		}
		memcpy(header->magic, data, 8);

		header->ctrllen = offtin(data + 8);
		header->datalen = offtin(data + 16);
		header->filelen = offtin(data + 24);

		if ((header->ctrllen < 0) || (header->datalen < 0) || (header->filelen < 0)) {
			error("Unable to read lengths in from binary patch header\n");
			bpatch_header_free(header);
			return NULL;
		}

	}
	return header;
}

void bpatch_header_free(bpatch_header_t* header) {
	if (header) {
		free(header);
	}
}

void bpatch_header_debug(bpatch_header_t* header) {
	debug("Header:\n");
	debug("\tmagic = %c%c%c%c%c%c%c%c\n", header->magic[0], header->magic[1], header->magic[2], header->magic[3],
											header->magic[4], header->magic[5], header->magic[6], header->magic[7]);
	debug("\tctrllen = %llu (compressed)\n", header->ctrllen);
	debug("\tdatalen = %llu (compressed)\n", header->datalen);
	debug("\tfilelen = %llu (uncompressed)\n", header->filelen);
	debug("\n");
}

int bpatch_decompress(bpatch_t* bpatch, uint8_t* input, uint64_t in_size, uint8_t* output, uint64_t* out_size) {
	int err = 0;
	unsigned int got = BUFSIZE;
	unsigned int was = *out_size;
	//unsigned int was = *out_size;
	//hexdump(input, in_size);
	err = BZ2_bzBuffToBuffDecompress(output, &got, input, in_size, 0, 0);
	if (err == BZ_OK) {
		debug("Unable to decompress buffer %d\n", err);
		*out_size = 0;
		return -1;
	}

	if (got < was && got <= 512) {
		//hexdump(output, got);

	} else {
		error("Unable to fill up decompression buffer\n");
		*out_size = 0;
		return -1;
	}
	*out_size = got;

	return 0;
}
