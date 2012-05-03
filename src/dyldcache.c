/**
 * GreenPois0n Absinthe - dyldcache.c
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

#include "file.h"
#include "debug.h"
#include "common.h"
#include "endianness.h"

#include "dyldmap.h"
#include "dyldimage.h"
#include "dyldcache.h"

/*
 * Dyldcache Functions
 */
dyldcache_t* dyldcache_create() {
	dyldcache_t* cache = (dyldcache_t*) malloc(sizeof(dyldcache_t));
	if (cache) {
		memset(cache, '\0', sizeof(dyldcache_t));
	}
	return cache;
}

dyldcache_t* dyldcache_open(const char* path) {
	int i = 0;
	int err = 0;
	uint32_t count = 0;
	uint32_t offset = 0;
	uint32_t length = 0;
	file_t* file = NULL;
	dyldcache_t* cache = NULL;
	dyldimage_t* image = NULL;
	unsigned char* data = NULL;
	unsigned char* buffer = NULL;

	cache = dyldcache_create();
	if (cache) {
		err = file_read(path, &buffer, &length);
		if (err < 0) {
			error("Unable to open file at path %s\n", path);
			dyldcache_free(cache);
			return NULL;
		}
		cache->data = buffer;
		cache->size = length;

		cache->header = dyldcache_header_load(cache);
		if (cache->header == NULL) {
			error("Unable to parse dyldcache header\n");
			dyldcache_free(cache);
			return NULL;
		}
		cache->count = cache->header->images_count;
		cache->offset = cache->header->images_offset;

		cache->arch = dyldcache_architecture_load(cache);
		if (cache->arch == NULL) {
			error("Unable to parse architecture from dyldcache header\n");
			dyldcache_free(cache);
			return NULL;
		}

		cache->maps = dyldcache_maps_load(cache);
		if (cache->maps == NULL) {
			error("Unable to load maps from dyldcache\n");
			dyldcache_free(cache);
			return NULL;
		}

		cache->images = dyldcache_images_load(cache);
		if (cache->images == NULL) {
			error("Unable to load images from dyldcache\n");
			dyldcache_free(cache);
			return NULL;
		}

		//dyldcache_debug(cache);
	}
	return cache;
}

void dyldcache_free(dyldcache_t* cache) {
	if (cache) {
		if (cache->header) {
			dyldcache_header_free(cache->header);
			cache->header = NULL;
		}
		if (cache->maps) {
			dyldcache_maps_free(cache->maps);
			cache->maps = NULL;
		}
		if (cache->images) {
			dyldcache_images_free(cache->images);
			cache->images = NULL;
		}
		if (cache->arch) {
			dyldcache_architecture_free(cache->arch);
			cache->arch = NULL;
		}
		if (cache->data) {
			free(cache->data);
			cache->data = NULL;
		}
		if(cache->file) {
			file_free(cache->file);
			cache->file = NULL;
		}
		free(cache);
	}
}

void dyldcache_debug(dyldcache_t* cache) {
	if (cache) {
		debug("Dyldcache:\n");
		if (cache->header) dyldcache_header_debug(cache->header);
		if (cache->images) dyldcache_images_debug(cache->images);
		if (cache->maps) dyldcache_maps_debug(cache->maps);
	}
}

/*
 * Dyldcache Architecture Functions
 */
architecture_t* dyldcache_architecture_create() {
	architecture_t* arch = (architecture_t*) malloc(sizeof(architecture_t));
	if (arch) {
		memset(arch, '\0', sizeof(architecture_t*));
	}
	return arch;
}

architecture_t* dyldcache_architecture_load(dyldcache_t* cache) {
	unsigned char* found = NULL;
	architecture_t* arch = dyldcache_architecture_create();
	if (arch) {
		found = strstr(cache->data, DYLDARCH_ARMV6);
		if (found) {
			arch->name = DYLDARCH_ARMV6;
			arch->cpu_type = kArmType;
			arch->cpu_subtype = kArmv6;
			arch->cpu_endian = kLittleEndian;
			return arch;
		}

		found = strstr(cache->data, DYLDARCH_ARMV7);
		if (found) {
			arch->name = DYLDARCH_ARMV7;
			arch->cpu_type = kArmType;
			arch->cpu_subtype = kArmv7;
			arch->cpu_endian = kLittleEndian;
			return arch;
		}

		// TODO: Add other architectures in here. We only need iPhone for now.
	}

	if (found == NULL) {
		error("Unknown architechure encountered! %s\n", cache->data);
	}

	return arch;
}

void dyldcache_architecture_debug(architecture_t* arch) {
	debug("\tArchitecture:\n");
	debug("\t\tname = %s\n", arch->name);
	debug("\t\tcpu_id = %d\n", arch->cpu_type);
	debug("\t\tcpu_sub_id = %d\n", arch->cpu_subtype);
	debug("\t\tcpu_endian = %s\n", arch->cpu_endian == kLittleEndian ? "little endian" : "big endian");
	debug("\n");
}

void dyldcache_architecture_free(architecture_t* arch) {
	if (arch) {
		free(arch);
	}
}

/*
 * Dyldcache Header Functions
 */
dyldcache_header_t* dyldcache_header_create() {
	dyldcache_header_t* header = (dyldcache_header_t*) malloc(sizeof(dyldcache_header_t));
	if (header) {
		memset(header, '\0', sizeof(dyldcache_header_t));
	}
	return header;
}

void dyldcache_header_free(dyldcache_header_t* header) {
	if (header) {
		free(header);
	}
}

dyldcache_header_t* dyldcache_header_load(dyldcache_t* cache) {
	dyldcache_header_t* header = dyldcache_header_create();
	if (header) {
		memcpy(header, cache->data, sizeof(dyldcache_header_t));
	}
	return header;
}

void dyldcache_header_debug(dyldcache_header_t* header) {
	debug("\tHeader:\n");
	debug("\t\tmagic = %s\n", header->magic);
	debug("\t\tmapping_offset = %u\n", header->mapping_offset);
	debug("\t\tmapping_count = %u\n", header->mapping_count);
	debug("\t\timages_offset = %u\n", header->images_offset);
	debug("\t\timages_count = %u\n", header->images_count);
	debug("\t\tbase_address = 0x%qX\n", header->base_address);
	debug("\t\tcodesign_offset = 0x%qX\n", header->codesign_offset);
	debug("\t\tcodesign_size = %llu\n", header->codesign_size);
	debug("\n");
}

/*
 * Dyldcache Images Functions
 */
dyldimage_t** dyldcache_images_create(uint32_t count) {
	uint32_t size = (count+1) * sizeof(dyldimage_t*);
	dyldimage_t** images = (dyldimage_t**) malloc(size);
	if (images) {
		memset(images, '\0', size);
	}
	return images;
}

dyldimage_t** dyldcache_images_load(dyldcache_t* cache) {
	uint32_t i = 0;
	uint32_t count = 0;
	uint32_t offset = 0;
	uint8_t* buffer = NULL;
	dyldimage_t* image = NULL;
	dyldimage_t** images = NULL;

	if (cache) {
		count = cache->header->images_count;
		images = dyldcache_images_create(count);
		if (images == NULL) {
			error("Unable to allocate memory for dyld images\n");
			return NULL;
		}

		for (i = 0; i < count; i++) {
			image = dyldimage_parse(cache->data, cache->header->images_offset + i*sizeof(dyldimage_info_t));
			if (image == NULL) {
				error("Unable to parse dyld image from cache\n");
				return NULL;
			}
			image->map = dyldcache_map_address(cache, image->address);
			image->offset = image->address - image->map->address;
			image->data = &cache->data[image->offset];
			image->size = *(uint32_t*)(image->data + 0x38);
			images[i] = image;
		}
		//dyldcache_images_debug(maps);
	}
	return images;
}

void dyldcache_images_debug(dyldimage_t** images) {
	if (images) {
		debug("\tImages:\n");
		int i = 0;
		while (images[i]) {
			dyldimage_debug(images[i]);
			i++;
		}
		debug("\n");
	}
}

void dyldcache_images_free(dyldimage_t** images) {
	if (images) {
		// Loop through each image and free it
		int i = 0;
		while (images[i]) {
			dyldimage_free(images[i]);
			i++;
		}
		free(images);
		images = NULL;
	}
}

/*
 * Dyldcache Maps Functions
 */
dyldmap_t** dyldcache_maps_create(uint32_t count) {
	uint32_t size = (count+1) * sizeof(dyldmap_t*);
	dyldmap_t** maps = (dyldmap_t**) malloc(size);
	if (maps) {
		memset(maps, '\0', size);
	}
	return maps;
}

dyldmap_t** dyldcache_maps_load(dyldcache_t* cache) {
	int i = 0;
	uint32_t count = 0;
	dyldmap_t** maps = NULL;
	if (cache) {
		count = cache->header->mapping_count;
		maps = dyldcache_maps_create(count);
		if (maps == NULL) {
			error("Unable to allocate memory for dyld maps\n");
			return NULL;
		}

		for (i = 0; i < count; i++) {
			maps[i] = dyldmap_parse(cache->data, cache->header->mapping_offset + i*sizeof(dyldmap_info_t));
			if (maps[i] == NULL) {
				error("Unable to parse dyld map from cache\n");
				return NULL;
			}
			cache->count++;
		}
		//dyldcache_maps_debug(maps);
	}
	return maps;
}

void dyldcache_maps_debug(dyldmap_t** maps) {
	if (maps) {
		debug("\tMaps:\n");
		int i = 0;
		while (maps[i]) {
			dyldmap_info_debug(maps[i]->info);
			i++;
		}
		debug("\n");
	}
}

void dyldcache_maps_free(dyldmap_t** maps) {
	if (maps) {
		int i = 0;
		// Loop through each map and free it
		while (maps[i]) {
			dyldmap_free(maps[i]);
			i++;
		}
		free(maps);
	}
}

dyldmap_t* dyldcache_map_image(dyldcache_t* cache, dyldimage_t* image) {
	return dyldcache_map_address(cache, image->address);
}

dyldmap_t* dyldcache_map_address(dyldcache_t* cache, uint64_t address) {
	int i = 0;
	dyldmap_t* map = NULL;
	for(i = 0; i < cache->header->mapping_count; i++) {
		map = cache->maps[i];
		if(dyldmap_contains(map, address) == kTrue) {
			return map;
		}
	}
	return NULL;
}


dyldimage_t* dyldcache_get_image(dyldcache_t* cache, const char* dylib) {
	int i = 0;
	dyldimage_t* image = NULL;
	for(i = 0; i < cache->count; i++) {
		image = cache->images[i];
		if(image != NULL) {
			printf("Found %s\n", image->name);
			if(!strcmp(image->name, dylib)) {
				return image;
			}
		}
	}
	return NULL;
}

dyldimage_t* dyldcache_first_image(dyldcache_t* cache) {
	return cache->images[0];
}

dyldimage_t* dyldcache_next_image(dyldcache_t* cache, dyldimage_t* image) {
	int i = 0;
	dyldimage_t* next = NULL;
	for(i = 0; i < cache->count; i++) {
		if(cache->images[i] == image) {
			next = cache->images[i+1];
			break;
		}
	}
	return next;
}
