/**
  * GreenPois0n Absinthe - stream.c
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

#include "stream.h"

#define BUFSIZE 4096

stream_t* stream_create() {
	stream_t* stream = (stream_t*) malloc(sizeof(stream_t));
	if (stream) {
		memset(stream, '\0', sizeof(stream_t));
	}
	return stream;
}

stream_t* stream_open(const char* path) {
	uint64_t got = 0;
	uint8_t buffer[4096];

	stream_t* stream = stream_create();
	if (stream) {
		stream->desc = fopen(path, "rb");
		if (stream->desc == NULL) {
			fprintf(stderr, "Unable to open stream %s\n", path);
			return NULL;
		}

		stream->path = strdup(path);
		if (stream->path == NULL) {
			fprintf(stderr, "Unable to allocate memory for stream path\n");
			stream_free(stream);
			return NULL;
		}

		fseek(stream->desc, 0, SEEK_END);
		stream->size = ftell(stream->desc);
		fseek(stream->desc, 0, SEEK_SET);

		stream->offset = 0;
		stream->data = (unsigned char*) malloc(stream->size);
		if (stream->data == NULL) {
			fprintf(stderr, "Unable to allocate memory for stream\n");
			stream_free(stream);
			return NULL;
		}

		uint64_t offset = 0;
		while (offset < stream->size) {
			memset(buffer, '\0', BUFSIZE);
			got = fread(buffer, 1, BUFSIZE, stream->desc);
			if (got > 0) {
				offset += got;
				memcpy(&(stream->data[offset]), buffer, got);
			} else {
				break;
			}
		}
		fprintf(stderr, "Read in %llu of %llu bytes from %s\n", stream->offset, stream->size, stream->path);
		// We have the data stored in memory now, so we don't need to keep this open anymore
		//fseek(stream->desc, 0, SEEK_SET);
		stream_close(stream);
		stream->offset = 0;
	}
	return stream;
}

void stream_close(stream_t* stream) {
	if (stream) {
		if (stream->desc) {
			fclose(stream->desc);
			stream->desc = NULL;
		}
	}
}

void stream_free(stream_t* stream) {
	if (stream) {
		if (stream->desc) {
			stream_close(stream);
			stream->desc = NULL;
		}
		if (stream->path) {
			free(stream->path);
			stream->path = NULL;
		}
		if(stream->data) {
			free(stream->data);
			stream->data = NULL;
		}
		stream->size = 0;
		stream->offset = 0;
		free(stream);
	}
}

unsigned int stream_read(stream_t* stream, unsigned char* data, unsigned int size) {
	int len = 0;
	if(stream->offset + size > stream->size) {
		len = stream->size - stream->offset;
	} else {
		len = size;
	}
	memcpy(data, &stream->data[stream->offset], len);
	return size;
}

unsigned int stream_write(stream_t* stream, unsigned char* data, unsigned int size) {
	int len = 0;
	if(stream->offset + size > stream->size) {
		len = stream->size - stream->offset;
	} else {
		len = size;
	}
	memcpy(&stream->data[stream->offset], data, len);
	return size;
}

long stream_tell(stream_t* stream) {
	return stream->offset;
}

void stream_seek(stream_t* stream, long offset) {
	stream->offset = offset;
}
