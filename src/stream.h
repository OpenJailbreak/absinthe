/**
  * GreenPois0n Absinthe - stream.h
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

#ifndef STREAM_H_
#define STREAM_H_

#include <stdint.h>

typedef struct stream_t {
	FILE* desc;
	char* path;
	uint64_t size;
	uint64_t offset;
	unsigned char* data;
} stream_t;

stream_t* stream_create();
stream_t* stream_open(const char* path);

void stream_close(stream_t* stream);
void stream_free(stream_t* stream);

unsigned int stream_read(stream_t* stream, unsigned char* data, unsigned int size);
unsigned int stream_write(stream_t* stream, unsigned char* data, unsigned int size);

long stream_tell(stream_t* stream);
void stream_seek(stream_t* stream, long offset);

#endif
