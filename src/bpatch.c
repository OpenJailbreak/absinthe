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

#define BUFSIZE 4096

static off_t offtin(unsigned char *buf) {
	off_t y;

	y = buf[7] & 0x7F;
	y <<= 8; y += buf[6];
	y <<= 8; y += buf[5];
	y <<= 8; y += buf[4];
	y <<= 8; y += buf[3];
	y <<= 8; y += buf[2];
	y <<= 8; y += buf[1];
	y <<= 8; y += buf[0];

	if(buf[7] & 0x80) y = -y;

	return y;
}

bpatch_t* bpatch_create() {
	bpatch_t* bpatch = (bpatch_t*) malloc(sizeof(bpatch_t));
	if(bpatch) {
		memset(bpatch, '\0', sizeof(bpatch_t));
	}
	bpatch->stream = (bz_stream*) malloc(sizeof(bz_stream));
	if(bpatch->stream != NULL) {
		memset(bpatch->stream, '\0', sizeof(bz_stream));
	}
	return bpatch;
}

bpatch_t* bpatch_open(const char* path) {
	size_t size = 0;
	uint8_t* data = NULL;
	file_read(path, &data, &size);
	if(data == NULL || size <= 0) {
		error("Unable to open binary patch file\n");
		return NULL;
	}

	bpatch_t* bpatch = bpatch_load(data, size);
	if(bpatch == NULL) {
		error("Unable to load binary patch file\n");
		return NULL;
	}

	bpatch->path = strdup(path);
	if(bpatch->path == NULL) {
		bpatch_free(bpatch);
		return NULL;
	}

	return bpatch;
}

void bpatch_free(bpatch_t* bpatch) {
	if(bpatch) {
		if(bpatch->stream) {
			BZ2_bzDecompressEnd(bpatch->stream);
			bpatch->stream = NULL;
		}
		if(bpatch->path) {
			free(bpatch->path);
			bpatch->path = NULL;
		}
		free(bpatch);
	}
}

bpatch_t* bpatch_load(unsigned char* data, unsigned int size) {
	bpatch_t* bpatch = bpatch_create();
	if(bpatch != NULL) {
			BZ2_bzDecompressInit(bpatch->stream, 0, kFalse);

			bpatch->stream->next_in = NULL;
			bpatch->stream->avail_in = 0;
			bpatch->stream->next_out = NULL;
			bpatch->stream->avail_out = 0;

			bpatch->header = bpatch_header_load(data, size);
			if(bpatch->header == NULL) {
				error("Unable to load binary patch header\n");
				bpatch_free(bpatch);
				return NULL;
			}

	}

	return bpatch;
}

void bpatch_debug(bpatch_t* bpatch) {
	if(bpatch) {
		debug("Binary Patch\n");
		if(bpatch->header) {
			bpatch_header_debug(bpatch->header);
		}
	}
}


unsigned int bpatch_read(bpatch_t* bpatch, unsigned char* data, unsigned int size) {
	/*
	int ended = 0;
	unsigned int toRead;
	unsigned int haveRead;
	unsigned int total;

	total = size;

	int bzerr = BZ_OK;

	while(total > 0) {
		if(bpatch->ended == kFalse) {
			hexdump(bpatch->input, bpatch->stream->avail_in);
			//memmove(bpatch->input, &bpatch->file->data[bpatch->offset], bpatch->stream->avail_in);
			bfile_read(bpatch->file, bpatch->input, size);
			hexdump(bpatch->input, bpatch->stream->avail_in);
			bfile_seek(bpatch->file, bpatch->offset);
			haveRead = bfile_read(bpatch->file, bpatch->input + bpatch->stream->avail_in, BUFSIZE - bpatch->stream->avail_in);
			bpatch->offset += haveRead;
			bpatch->stream->avail_in += haveRead;
			bpatch->stream->next_in = (char*) bpatch->input;

			bzerr = BZ2_bzDecompress(bpatch->stream);

			if(bzerr == BZ_STREAM_END) {
				ended = 1;
			} else {
				if(bzerr != BZ_OK) {
					return 0;
				}
			}
		}

		if(total > (BUFSIZE - bpatch->stream->avail_out)) {
			toRead = BUFSIZE - bpatch->stream->avail_out;
		} else {
			toRead = total;
		}

		memcpy(data, bpatch->output, toRead);
		memmove(bpatch->output, bpatch->output + toRead, bpatch->size - toRead);
		bpatch->stream->next_out -= toRead;
		bpatch->stream->avail_out += toRead;
		data += toRead;
		total -= toRead;

		if(total > 0 && ended) {
			return (size - total);
		}
	}

	return size;
	*/
}

int bpatch_apply(bpatch_t* bpatch, const char* path) {
	uint8_t* data = NULL;
	size_t* size = NULL;
	if(path != NULL) {
		file_read(path, &data, &size);
		if(data == NULL || size <= 0) {
			error("Unable to read target file %s\n");
			return -1;
		}

		// CleanUp
		free(data);
	}
	return 0;
}

/*
 * Binary Patch Header
 */
bpatch_header_t* bpatch_header_create() {
	bpatch_header_t* header = (bpatch_header_t*) malloc(sizeof(bpatch_header_t));
	if(header) {
		memset(header, '\0', sizeof(bpatch_header_t));
	}
	return header;
}

bpatch_header_t* bpatch_header_load(unsigned char* data, unsigned int size) {
	bpatch_header_t* header = bpatch_header_create();
	if(header != NULL) {
		memcpy(header, data, sizeof(bpatch_header_t));

		if (memcmp(header->magic, "BSDIFF40", 8) != 0) {
			error("Unable to find magic string in binary patch header");
			bpatch_header_free(header);
			return NULL;
		}

		header->ctrllen = offtin(data + 8);
		header->datalen = offtin(data + 16);
		header->filelen = offtin(data + 24);

		if((header->ctrllen < 0) || (header->datalen < 0) || (header->filelen < 0)) {
			error("Unable to read lengths in from binary patch header\n");
			bpatch_header_free(header);
			return NULL;
		}

	}
	return header;
}

void bpatch_header_free(bpatch_header_t* header) {
	if(header) {
		free(header);
	}
}

void bpatch_header_debug(bpatch_header_t* header) {
	debug("Header:\n");
	debug("\tmagic = %c%c%c%c%c%c%c%c\n", header->magic[0], header->magic[1], header->magic[2], header->magic[3],
											header->magic[4], header->magic[5], header->magic[6], header->magic[7]);
	debug("\tctrllen = %llu\n", header->ctrllen);
	debug("\tdatalen = %llu\n",  header->datalen);
	debug("\tfilelen = %llu\n", header->filelen);
	debug("\n");
}

