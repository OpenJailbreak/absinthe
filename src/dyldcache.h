/**
  * GreenPois0n Absinthe - dyldcache.h
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

#ifndef DYLDCACHE_H_
#define DYLDCACHE_H_

#include <stdint.h>

#define DYLDCACHE_DIR "/var/db/dyld"
#define DYLDCACHE_NAME "dyld_shared_cache"

typedef struct dyldcache_header_t {
	char magic[16];
	uint32_t mapping_offset;
	uint32_t mapping_count;
	uint32_t images_offset;
	uint32_t images_count;
	uint64_t base_address;
	uint64_t codesign_offset;
	uint64_t codesign_size;
} dyldcache_header_t;

typedef struct dyldcache_t {
	dyldcache_header_t* header;
} dyldcache_t;

dyldcache_t* dyldcache_create();
void dyldcache_debug(dyldcache_t* cache);
void dyldcache_free(dyldcache_t* cache);
dyldcache_t* dyldcache_open(const char* path);

dyldcache_header_t* dyldcache_header_create();
void dyldcache_header_free(dyldcache_header_t* header);
void dyldcache_header_debug(dyldcache_header_t* header);
dyldcache_header_t* dyldcache_header_parse(unsigned char* data);

#endif /* DYLDCACHE_H_ */
