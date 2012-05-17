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

static off_t offtin(uint8_t* buf) {
	off_t y;

	y = buf[7] & 0x7F;
	y = y * 256;
	y += buf[6];
	y = y * 256;
	y += buf[5];
	y = y * 256;
	y += buf[4];
	y = y * 256;
	y += buf[3];
	y = y * 256;
	y += buf[2];
	y = y * 256;
	y += buf[1];
	y = y * 256;
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
	uint64_t size = 0;
	uint8_t* data = NULL;
	file_read(path, &data, &size);
	if (data == NULL || size == 0) {
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

bpatch_t* bpatch_load(uint8_t* data, int64_t size) {
	int err = 0;
	uint8_t* buf = NULL;
	int64_t offset = 0;
	int64_t buf_size = 0;
	int64_t data_size = 0;
	int64_t extra_size = 0;
	int64_t control_size = 0;

	bpatch_t* bpatch = bpatch_create();
	if (bpatch != NULL) {
		bpatch->header = bpatch_header_load(&data[offset],
				sizeof(bpatch_header_t));
		if (bpatch->header == NULL) {
			error("Unable to load binary patch header\n");
			bpatch_free(bpatch);
			return NULL;
		}
		offset += sizeof(bpatch_header_t);

		buf = malloc(BUFSIZE + 1);
		if (buf == NULL) {
			error("Unable to allocate buffer\n");
			return NULL;
		}

		//////////////////////////
		// Load in control block
		control_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(&data[offset], bpatch->header->ctrllen, buf,
				&control_size);
		if (err < 0 || control_size <= 0) {
			error("Unable to decompress control block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		// Make sure there's enough data left
		if (bpatch->header->ctrllen + offset > size) {
			error("Sanity check failed\n");
			bpatch_free(bpatch);
			return NULL;
		}

		// Allocate memory for our decompressed control block
		bpatch->control_size = control_size;
		bpatch->control = (uint8_t*) malloc(control_size + 1);
		if (bpatch->control == NULL) {
			error("Unable to allocate control block\n");
			bpatch_free(bpatch);
			return NULL;
		}memset(bpatch->control, '\0', control_size);
		memcpy(bpatch->control, buf, control_size);
		offset += bpatch->header->ctrllen;

		//////////////////////////
		// Load in diff block
		data_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(&data[offset], bpatch->header->datalen, buf,
				&data_size);
		if (err < 0 || data_size <= 0) {
			error("Unable to decompress diff block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		// Allocate memory for our diff block
		bpatch->data_size = data_size;
		bpatch->data = malloc(data_size + 1);
		if (bpatch->data == NULL) {
			error("Unable to allocate memory for diff block\n");
			return NULL;
		}memset(bpatch->data, '\0', data_size);
		memcpy(bpatch->data, buf, data_size);
		offset += bpatch->header->datalen;

		////////////////////////
		// Load in extra block
		extra_size = BUFSIZE;
		memset(buf, '\0', BUFSIZE);
		err = bpatch_decompress(&data[offset], (size - offset), buf,
				&extra_size);
		if (err < 0 || extra_size <= 0) {
			error("Unable to decompress extra block\n");
			bpatch_free(bpatch);
			return NULL;
		}

		bpatch->extra_size = extra_size;
		bpatch->extra = malloc(extra_size + 1);
		if (bpatch->extra == NULL) {
			error("Unable to allocate memory for extra block\n");
			bpatch_free(bpatch);
			return NULL;
		}memset(bpatch->extra, '\0', BUFSIZE);
		memcpy(bpatch->extra, buf, extra_size);
		offset += extra_size;
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
			debug("Control:\n");debug("\tsize = %llu\n", bpatch->control_size);
			if (bpatch->control != NULL) {
				//debug("compressed: \n");
				//hexdump(&bpatch->data[sizeof(bpatch_header_t)], bpatch->header->ctrllen);
				//debug("decompressed: \n");
				//hexdump(bpatch->control, bpatch->control_size);
			}
		}
		if (bpatch->data_size > 0 && bpatch->data_size < BUFSIZE) {
			debug("Data:\n");debug("\tsize = %llu\n", bpatch->data_size);
			if (bpatch->data != NULL && bpatch->data_size <= 512) {
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
		if (source_data == NULL || source_size == 0) {
			error("Unable to read target file %s\n", path);
			return -1;
		}

		target_data = (uint8_t*) malloc(target_size + 1);
		if (target_data == NULL) {
			error("Unable to allocate data for new file\n");
			return -1;
		}memset(target_data, '\0', target_size);

		int8_t* ctrl_start = ctrl_data;
		int8_t* ctrl_end = ctrl_start + ctrl_size;
		int8_t* ctrl_position = ctrl_start + ctrl_size;
		while (ctrl_start + ctrl_offset < ctrl_start + ctrl_size) {
			// Loop 3 times to read in X, Y, and Z values from the control vector
			for (i = 0; i <= 2; i++) {
				ctrl[i] = offtin(&bpatch->control[ctrl_offset]);
				ctrl_offset += 8;
			}
			x = ctrl[0];
			y = ctrl[1];
			z = ctrl[2];
			debug("x = %qd, y = %qd, z = %qd\n", x, y, z);

			//if(source_offset + x > source_size) {
			//	error("Fail sanitary check\n");
			//	return -1;
			//}

			for (i = 0; i < x; i++) {
				uint8_t value1 = source_data[source_offset + i];
				uint8_t value2 = bpatch->data[data_offset + i];
				target_data[target_offset + i] = (value1 + value2) & 0xFF;
			}

			data_offset += x;
			target_offset += x;
			source_offset += x;

			//hexdump(&target_data[target_offset], 0x200);
			for (i = 0; i < y; i++) {
				uint8_t value1 = bpatch->extra[extra_offset + i];
				uint8_t value2 = bpatch->data[data_offset + i];
				target_data[target_offset + i] = value1;
			}
			//hexdump(&target_data[target_offset], 0x200);

			extra_offset += y;
			target_offset += y;
			source_offset += z;

		}

		// CleanUp
		file_write(path, target_data, target_size);
	}
	if (target_data)
		free(target_data);
	if (target_data)
		free(source_data);
	return 0;
}

/*
 * Binary Patch Header
 */
bpatch_header_t* bpatch_header_create() {
	bpatch_header_t* header = (bpatch_header_t*) malloc(sizeof(bpatch_header_t));
	if (header) {
		memset(header, '\0', sizeof(bpatch_header_t));
	}
	return header;
}

bpatch_header_t* bpatch_header_load(uint8_t* data, int64_t size) {
	bpatch_header_t* header = bpatch_header_create();
	if (header != NULL) {
		if (memcmp(data, "BSDIFF40", 8) != 0) {
			error("Unable to find magic string in binary patch header");
			bpatch_header_free(header);
			return NULL;
		}memcpy(header->magic, data, 8);

		header->ctrllen = offtin(data + 8);
		header->datalen = offtin(data + 16);
		header->filelen = offtin(data + 24);

		if ((header->ctrllen < 0) || (header->datalen < 0)
				|| (header->filelen < 0)) {
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
	debug("Header:\n"); debug("\tmagic = %c%c%c%c%c%c%c%c\n", header->magic[0], header->magic[1], header->magic[2], header->magic[3],
			header->magic[4], header->magic[5], header->magic[6], header->magic[7]); debug("\tctrllen = %llu (compressed)\n", header->ctrllen); debug("\tdatalen = %llu (compressed)\n", header->datalen); debug("\tfilelen = %llu (uncompressed)\n", header->filelen); debug("\n");
}

unsigned int bpatch_decompress(char* input, unsigned int in_size, char* output,
		unsigned int* out_size) {
	int err = 0;
	char* dest = input;
	char* source = output;
	unsigned int was = *out_size;
	unsigned int size = in_size;
	unsigned int got = BUFSIZE;

	err = BZ2_bzBuffToBuffDecompress(output, out_size, input, in_size, 0, 0);
	got = *out_size;
	if (err != BZ_OK) {
		debug("Unable to decompress buffer %d\n", err);
		got = 0;
	}

	if (got != 0) {
		if (got < was) {
			//debug("bingo!!\n");

		} else {
			error("Unable to fill up decompression buffer\n");
			got = 0;
		}
	}
	*out_size = got;
	return got;
}
