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

static off_t offtin(unsigned char *buf) {
	off_t y;

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
	bpatch->stream = (bz_stream*) malloc(sizeof(bz_stream));
	if (bpatch->stream != NULL) {
		memset(bpatch->stream, '\0', sizeof(bz_stream));
	}
	return bpatch;
}

bpatch_t* bpatch_open(const char* path) {
	size_t size = 0;
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
		if (bpatch->stream) {
			//BZ2_bzDecompressEnd(bpatch->stream);
			bpatch->stream = NULL;
		}
		if (bpatch->path) {
			free(bpatch->path);
			bpatch->path = NULL;
		}
		free(bpatch);
	}
}

bpatch_t* bpatch_load(unsigned char* data, unsigned int size) {
	int err = 0;
	unsigned int offset = 0;
	unsigned int data_size = 0;
	unsigned int extra_size = 0;
	unsigned int control_size = 0;

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

		// Initialize bzip2
		//err = BZ2_bzDecompressInit(bpatch->stream, 0, kFalse);
		//if(err != BZ_OK) {
		//	error("Unable to initialize bzip2 error %d\n", err);
		//	bpatch_free(bpatch);
		//	return NULL;
		//}

		bpatch->stream->next_in = NULL;
		bpatch->stream->avail_in = 0;
		bpatch->stream->next_out = NULL;
		bpatch->stream->avail_out = 0;


		// Load in control block
		control_size = BUFSIZE;
		bpatch->control = malloc(BUFSIZE + 1);
		memset(bpatch->control, '\0', BUFSIZE);
		//hexdump(&data[offset], bpatch->header->ctrllen);
		err = bpatch_decompress(bpatch, &data[offset], bpatch->header->ctrllen,
				bpatch->control, &control_size);
		//hexdump(bpatch->control, control_size);
		if (err < 0 || control_size <= 0) {
			error("Oh no\n");
			// TODO: Give me a real error
			bpatch_free(bpatch);
			return NULL;
		} // Make sure there's enough data left
		  if(offset + bpatch->header->ctrllen > size) {
		  	error("Oh no\n"); // TODO: Give me a real error
		  	bpatch_free(bpatch);
		  	return NULL;
		  }
		bpatch->control_size = control_size;
		offset += bpatch->header->ctrllen;

		// Load in data block
		data_size = BUFSIZE;
		bpatch->data = malloc(BUFSIZE + 1);
		memset(bpatch->data, '\0', BUFSIZE);
		//hexdump(&data[offset], bpatch->header->datalen);
		err = bpatch_decompress(bpatch, &data[offset], bpatch->header->datalen,
				bpatch->data, &data_size);
		//hexdump(bpatch->data, data_size);
		if (err < 0 || data_size <= 0) {
			error("Oh no\n");
			// TODO: Give me a real error
			bpatch_free(bpatch);
			return NULL;
		} // Make sure there's enough data left
		  //if(offset + bpatch->header->datalen > size) {
		  //	error("Oh no\n"); // TODO: Give me a real error
		  //	bpatch_free(bpatch);
		  //	return NULL;
		  //}
		bpatch->data_size = data_size;
		offset += bpatch->header->datalen;

		// Load in extra block
		extra_size = BUFSIZE;
		bpatch->extra = malloc(BUFSIZE + 1);
		memset(bpatch->extra, '\0', BUFSIZE);
		hexdump(&data[offset], (size - offset));
		err = bpatch_decompress(bpatch, &data[offset], (size - offset),
				bpatch->extra, &extra_size);
		hexdump(bpatch->extra, extra_size);
		if (err < 0 || extra_size <= 0) {
			error("Oh no\n");
			// TODO: Give me a real error
			bpatch_free(bpatch);
			return NULL;
		} // Make sure there's enough data left
		  //if(offset + bpatch->header->datalen > size) {
		  //	error("Oh no\n"); // TODO: Give me a real error
		  //	bpatch_free(bpatch);
		  //	return NULL;
		  //}
		bpatch->extra_size = extra_size;

		//if (offset + extra_size != size) {
			//error("offset = %llu, extra_size = %d, size = %d\n");
		//}
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

unsigned int bpatch_read(bpatch_t* bpatch, unsigned char* data,
		unsigned int size) {
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
	int i = 0;
	int done = 0;
	int64_t ctrl[3];
	uint32_t got = 0;

	int64_t x, y, z;
	int8_t* old_data = NULL;
	int64_t old_size = 0;
	int64_t old_offset = 0;
	int8_t* new_data = NULL;
	int64_t new_offset = 0;
	int64_t new_size = bpatch->header->filelen;
	int64_t ctrl_offset = 0;
	int8_t* ctrl_data = bpatch->control;
	int64_t ctrl_size = bpatch->control_size;
	int8_t* cur_data = old_data;

	if (path != NULL) {
		file_read(path, &old_data, &old_size);
		if (old_data == NULL || old_size <= 0) {
			error("Unable to read target file %s\n", path);
			return -1;
		}

		uint8_t* new_data = malloc(new_size);
		if(new_data == NULL) {
			error("Unable to allocate data for new file\n");
			return -1;
		}
		memcpy(new_data, bpatch->data, bpatch->data_size);

		while ((ctrl_data + ctrl_offset) < (ctrl_data + ctrl_size)) {
			//info("0x%x / 0x%x\n", new_offset, new_size);
			for (i = 0; i <= 2; i++) {
				info("Loading in control block number %d at offset %qx\n", i, ctrl_offset);
				ctrl[i] = offtin(bpatch->control + ctrl_offset);
				//hexdump(ctrl + offset, 0x20);
				ctrl_offset += 8;
			}
			x = ctrl[0];y = ctrl[1];z = ctrl[2];
			info("x = %qd, y = %qd, z = %qd\n", x, y, z);

			if(x != 0) {
				memcpy(&new_data[new_offset], &old_data[old_offset], x);
				debug("0x%qx:\tCopying %qd bytes from old file to new file\n", old_offset, x);
				hexdump(&new_data[new_offset], 0x200);
				new_offset += x;
				old_offset += x;
			}
			if(y != 0) {
				memcpy(&new_data[new_offset], bpatch->extra, y);
				debug("0x%qx:\tCopying %qd bytes from extra block into new file\n", old_offset, y);
				//hexdump()
			}
			if(z != 0) {
				debug("0x%qx:\tSeeking to offset 0x%qx in the old file\n", old_offset, old_offset+z);
				old_offset += z;
			}
			//hexdump(ctrl, sizeof(ctrl));
		}

		// CleanUp
		free(old_data);
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

bpatch_header_t* bpatch_header_load(unsigned char* data, unsigned int size) {
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
			header->magic[4], header->magic[5], header->magic[6], header->magic[7]);
	debug("\tctrllen = %llu (compressed)\n", header->ctrllen);
	debug("\tdatalen = %llu (compressed)\n", header->datalen);
	debug("\tfilelen = %llu (uncompressed)\n", header->filelen);
	debug("\n");
}

int bpatch_decompress(bpatch_t* bpatch, char* input, unsigned int in_size,
		char* output, unsigned int* out_size) {
	int err = 0;
	unsigned int got = BUFSIZE;
	unsigned int was = *out_size;
	//unsigned int was = *out_size;
	//hexdump(input, in_size);
	err = BZ2_bzBuffToBuffDecompress(output, &got, input, in_size, 0, 0);
	if (err == BZ_OK) {
		if (got < was) {
			if(got <= 512) {
				//hexdump(output, got);
			}
			*out_size = got;
		} else {
			error("Unable to fill up decompression buffer\n");
			*out_size = 0;
		}
	} else {
		debug("Unable to decompress buffer %d\n", err);
		*out_size = 0;
	}
	return 0;
}
