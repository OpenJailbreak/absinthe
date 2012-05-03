/**
 * GreenPois0n Absinthe - dyldimage.c
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

#include "debug.h"
#include "common.h"
#include "dyldmap.h"
#include "dyldimage.h"

/*
 * Dyld Image Functions
 */

dyldimage_t* dyldimage_create() {
	dyldimage_t* image = (dyldimage_t*) malloc(sizeof(dyldimage_t));
	if (image) {
		memset(image, '\0', sizeof(dyldimage_t));
	}
	return image;
}

dyldimage_t* dyldimage_parse(unsigned char* data, uint32_t offset) {
	unsigned char* buffer = &data[offset];
	dyldimage_t* image = dyldimage_create();
	if (image) {
		image->info = dyldimage_info_parse(data, offset);
		if(image->info == NULL) {
			error("Unable to allocate data for dyld image info\n");
			return NULL;
		}
		image->path = &data[image->info->offset];
		image->name = strrchr(image->path, '/')+1;
		image->address = image->info->address;
		image->size = 0;
	}
	return image;
}

void dyldimage_free(dyldimage_t* image) {
	if (image) {
		if (image->info) {
			dyldimage_info_free(image->info);
			image->info = NULL;
		}
		free(image);
	}
}

void dyldimage_debug(dyldimage_t* image) {
	if (image) {
		debug("\t\tImage {\n");
		if (image->info) {
			dyldimage_info_debug(image->info);
		}
		debug("\t\t}\n");
	}
}

/*
 * Dyld Image Info Functions
 */

dyldimage_info_t* dyldimage_info_create() {
	dyldimage_info_t* info = (dyldimage_info_t*) malloc(sizeof(dyldimage_info_t));
	if(info) {
		memset(info, '\0', sizeof(dyldimage_info_t*));
	}
	return info;
}

dyldimage_info_t* dyldimage_info_parse(unsigned char* data, uint32_t offset) {
	dyldimage_info_t* info = dyldimage_info_create();
	if(info) {
		memcpy(info, &data[offset], sizeof(dyldimage_info_t));
	}
	return info;
}

void dyldimage_info_free(dyldimage_info_t* info) {
	if (info) {
		free(info);
	}
}
void dyldimage_info_debug(dyldimage_info_t* info) {
	if (info) {
		debug("\t\t\taddress = 0x%qx\n", info->address);
		debug("\t\t\t  inode = 0x%qx\n", info->inode);
		debug("\t\t\tmodtime = 0x%qx\n", info->modtime);
		debug("\t\t\t offset = 0x%08x\n", info->offset);
		debug("\t\t\t    pad = 0x%08x\n", info->pad);
	}
}

void dyldimage_save(dyldimage_t* image, const char* path) {
	if(image != NULL && image->data != NULL && image->size > 0) {
		printf("Writing dylib to %s\n", path);
		file_write(path, image->data, image->size);
	}
}

char* dyldimage_get_name(dyldimage_t* image) {
	return image->name;
}
