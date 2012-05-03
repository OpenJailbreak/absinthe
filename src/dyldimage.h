/**
  * GreenPois0n Absinthe - dyldimage.h
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

#ifndef DYLDIMAGE_H_
#define DYLDIMAGE_H_

#include <stdint.h>

#include "dyldmap.h"

typedef struct dyldimage_info_t {
	uint64_t address;
	uint64_t modtime;
	uint64_t inode;
	uint32_t offset;
	uint32_t pad;
} dyldimage_info_t;

typedef struct dyldimage_t {
	char* name;
	char* path;
	uint8_t* data;
	uint32_t size;
	uint32_t index;
	uint32_t offset;
	uint64_t address;
	dyldmap_t* map;
	dyldimage_info_t* info;
} dyldimage_t;

/*
 * Dyld Image Functions
 */
dyldimage_t* dyldimage_create();
dyldimage_t* dyldimage_parse(unsigned char* data, uint32_t offset);
char* dyldimage_get_name(dyldimage_t* image);
void dyldimage_save(dyldimage_t* image, const char* path);
void dyldimage_free(dyldimage_t* image);
void dyldimage_debug(dyldimage_t* image);

/*
 * Dyld Image Info Functions
 */
dyldimage_info_t* dyldimage_info_create();
dyldimage_info_t* dyldimage_info_parse(unsigned char* data, uint32_t offset);
void dyldimage_info_free(dyldimage_info_t* info);
void dyldimage_info_debug(dyldimage_info_t* info);

#endif /* DYLDIMAGE_H_ */
