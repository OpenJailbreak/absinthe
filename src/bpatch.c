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

#include <bzlib.h>

#include "bpatch.h"

#define BUFFERSIZE 1024

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

unsigned int bpatch_read(bpatch_t* bpatch, int *bzerr, unsigned char* out, unsigned int len) {
	int ended = 0;
	unsigned int toRead;
	unsigned int haveRead;
	unsigned int total;

	total = len;

	*bzerr = BZ_OK;

	while(total > 0) {
		if(!ended) {
			memmove(bpatch->input, bpatch->stream->next_in, bpatch->stream->avail_in);
			//bpatch->file->seek(bpatch->file, bpatch->offset);
			//haveRead = bpatch->file->read(bpatch->file, bpatch->input + bpatch->stream.avail_in, bpatch->bufferLen - bpatch->stream.avail_in);
			bpatch->offset += haveRead;
			bpatch->stream->avail_in += haveRead;
			bpatch->stream->next_in = (char*) bpatch->input;

			*bzerr = BZ2_bzDecompress(bpatch->stream);

			if(*bzerr == BZ_STREAM_END) {
				ended = 1;
			} else {
				if(*bzerr != BZ_OK) {
					return 0;
				}
			}
		}

		if(total > (bpatch->size - bpatch->stream->avail_out)) {
			//toRead = bpatch->bufferLen - bpatch->stream.avail_out;
		} else {
			toRead = total;
		}

		memcpy(out, bpatch->output, toRead);
		//memmove(bpatch->output, bpatch->output + toRead, bpatch->bufferLen - toRead);
		bpatch->stream->next_out -= toRead;
		bpatch->stream->avail_out += toRead;
		out += toRead;
		total -= toRead;

		if(total > 0 && ended) {
			return (len - total);
		}
	}

	return len;
}

int bpatch(const char* in, const char* out, const char* patch) {
	return 0;
}

bpatch_t* bpatch_create() {
	bpatch_t* bpatch = (bpatch_t*) malloc(sizeof(bpatch_t));
	if(bpatch) {
		memset(bpatch, '\0', sizeof(bpatch_t));
	}
	return bpatch;
}

bpatch_t* bpatch_open(const char* path) {
	bpatch_t* bpatch = bpatch_create();
	if(bpatch != NULL) {
		bpatch->path = strdup(path);
		if(bpatch->path == NULL) {
			bpatch_free(bpatch);
			return NULL;
		}

		file_read(path, &bpatch->data, &bpatch->size);
		if(bpatch->data == NULL || bpatch->size <= 0) {
			bpatch_free(bpatch);
			return NULL;
		}
		/*
	bpatch_t* stream;
	stream = (bpatch_t*) malloc(sizeof(bpatch_t));
	stream->file = file;
	stream->offset = offset;
	stream->bufferLen = bufferLen;
	stream->inBuffer = (unsigned char*) malloc(bufferLen);
	stream->outBuffer = (unsigned char*) malloc(bufferLen);
	memset(&(stream->bz2), 0, sizeof(bz_stream));
	BZ2_bzDecompressInit(&(stream->bz2), 0, FALSE);

	stream->bz2.next_in = (char*) stream->inBuffer;
	stream->bz2.avail_in = 0;
	stream->bz2.next_out = (char*) stream->outBuffer;
	stream->bz2.avail_out = bufferLen;

	stream->ended = FALSE;
	return stream;
		 */
	}

	return bpatch;
}

int bpatch_apply(bpatch_t* patch, const char* path) {
	unsigned char header[32], buf[8];
	off_t oldsize, newsize;
	off_t bzctrllen, bzdatalen;
	off_t oldpos, newpos;
	int i;
	int cbz2err, dbz2err, ebz2err;
	off_t ctrl[3];
	unsigned int lenread;

	bpatch_t* cpfbz2;
	bpatch_t* dpfbz2;
	bpatch_t* epfbz2;

	/* Read header */
	/*
	if (patch->read(patch, header, 32) < 32) {
		return -1;
	}
	*/

	/* Check for appropriate magic */
	if (memcmp(header, "BSDIFF40", 8) != 0)
		return -2;

	/* Read lengths from header */
	bzctrllen = offtin(header + 8);
	bzdatalen = offtin(header + 16);
	newsize = offtin(header + 24);

	if((bzctrllen < 0) || (bzdatalen < 0) || (newsize < 0))
		return -3;

	//cpfbz2 = openbpatch_t(patch, 32, 1024);
	//dpfbz2 = openbpatch_t(patch, 32 + bzctrllen, 1024);
	//epfbz2 = openbpatch_t(patch, 32 + bzctrllen + bzdatalen, 1024);

	//oldsize = in->getLength(in);

	oldpos = 0;
	newpos = 0;
	unsigned char* writeBuffer = (unsigned char*) malloc(BUFFERSIZE);
	unsigned char* readBuffer = (unsigned char*) malloc(BUFFERSIZE);

	while(newpos < newsize) {
		/* Read control data */
		for(i=0;i<=2;i++) {
			//lenread = bpatch_read(&cbz2err, cpfbz2, buf, 8);
			if ((lenread < 8) || ((cbz2err != BZ_OK) &&
			    (cbz2err != BZ_STREAM_END)))
				return -4;
			ctrl[i] = offtin(buf);
		};

		/* Sanity-check */
		if((newpos + ctrl[0]) > newsize)
			return -5;

		/* Read diff string */
		unsigned int toRead;
		unsigned int total = ctrl[0];
		while(total > 0) {
			if(total > BUFFERSIZE)
				toRead = BUFFERSIZE;
			else
				toRead = total;

			memset(writeBuffer, 0, toRead);
			lenread = BZ2_bzRead(&dbz2err, dpfbz2, writeBuffer, toRead);
			if ((lenread < toRead) ||
			    ((dbz2err != BZ_OK) && (dbz2err != BZ_STREAM_END)))
				return -6;

			/* Add old data to diff string */
			//in->seek(in, oldpos);
			unsigned int maxRead;
			if((oldpos + toRead) > oldsize)
				maxRead = (oldsize > oldpos) ? oldsize - oldpos : 0;
			else
				maxRead = toRead;

			//in->read(in, readBuffer, maxRead);
			for(i = 0; i < maxRead; i++) {
				writeBuffer[i] += readBuffer[i];
			}

			//out->seek(out, newpos);
			//out->write(out, writeBuffer, toRead);

			/* Adjust pointers */
			newpos += toRead;
			oldpos += toRead;
			total -= toRead;
		}

		/* Sanity-check */
		if((newpos + ctrl[1]) > newsize)
			return -7;

		total = ctrl[1];

		while(total > 0){
			if(total > BUFFERSIZE)
				toRead = BUFFERSIZE;
			else
				toRead = total;

			/* Read extra string */
			lenread = BZ2_bzRead(&ebz2err, epfbz2, writeBuffer, toRead);
			if ((lenread < toRead) ||
			    ((ebz2err != BZ_OK) && (ebz2err != BZ_STREAM_END)))
				return -8;

			//out->seek(out, newpos);
			//out->write(out, writeBuffer, toRead);

			/* Adjust pointers */
			newpos += toRead;
			total -= toRead;
		}

		oldpos += ctrl[2];
	};

	free(writeBuffer);
	free(readBuffer);

	//closebpatch_t(cpfbz2);
	//closebpatch_t(dpfbz2);
	//closebpatch_t(epfbz2);

	//out->close(out);
	//in->close(in);

	//patch->close(patch);
	return 0;
}



void bpatch_free(bpatch_t* bpatch) {
	if(bpatch) {
		if(bpatch->stream) {
			BZ2_bzDecompressEnd(bpatch->stream);
			bpatch->stream = NULL;
		}
		if(bpatch->data) {
			free(bpatch->data);
			bpatch->size = 0;
			bpatch->data = NULL;
		}
		if(bpatch->path) {
			free(bpatch->path);
			bpatch->path = NULL;
		}
		free(bpatch);
	}
}
