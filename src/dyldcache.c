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

#include"file.h"
#include "dyldcache.h"

/*
 * Dyldcache Functions
 */
dyldcache_t* dyldcache_create() {
	dyldcache_t* cache = (dyldcache_t*) malloc(sizeof(dyldcache_t));
	if(cache) {
		memset(cache, '\0', sizeof(dyldcache_t));
	}
	return cache;
}

dyldcache_t* dyldcache_open(const char* path) {
	file_t* file = NULL;
	dyldcache_t* cache = NULL;

	cache = dyldcache_create();
	if(cache) {
		file = file_open(path);
		cache->header = dyldcache_header_parse(file->data);
		if(cache->header == NULL) {
			fprintf(stderr, "Unable to parse dyldcache header\n");
			return NULL;
		}
		dyldcache_debug(cache);
	}
	return cache;
}

void dyldcache_free(dyldcache_t* cache) {
	if(cache) {
		if(cache->header) {
			dyldcache_header_free(cache->header);
			cache->header = NULL;
		}
		free(cache);
	}
}

void dyldcache_debug(dyldcache_t* cache) {
	if(cache) {
		printf("Dyldcache:\n");
		if(cache->header) {
			dyldcache_header_debug(cache->header);
		}
	}
}

/*
 * Dyldcache Header Functions
 */
dyldcache_header_t* dyldcache_header_create() {
	dyldcache_header_t* header = (dyldcache_header_t*) malloc(sizeof(dyldcache_header_t));
	if(header) {
		memset(header, '\0', sizeof(header));
	}
	return header;
}

void dyldcache_header_free(dyldcache_header_t* header) {
	if(header) {
		free(header);
	}
}

dyldcache_header_t* dyldcache_header_parse(unsigned char* data) {
	dyldcache_header_t* header = dyldcache_header_create();
	if(header) {
		memcpy(header, data, sizeof(dyldcache_header_t));
	}
	return header;
}

void dyldcache_header_debug(dyldcache_header_t* header) {
	printf("\tHeader:\n");
	printf("\t\tmagic = %s\n", header->magic);
	printf("\t\tmapping_offset = %du\n", header->mapping_offset);
	printf("\t\tmapping_count = %du\n", header->mapping_count);
	printf("\t\timages_offset = %du\n", header->images_offset);
	printf("\t\timages_count = %du\n", header->images_count);
	printf("\t\tbase_address = %llu\n", header->base_address);
	printf("\t\tcodesign_offset = %llu\n", header->codesign_offset);
	printf("\t\tcodesign_size = %llu\n", header->codesign_size);
	printf("\n");
}
