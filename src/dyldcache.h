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

#include "common.h"
#include "endianness.h"

#define DYLDCACHE_DIR "/var/db/dyld"
#define DYLDCACHE_NAME "dyld_shared_cache"

#define DYLDARCH_PPC     "ppc"
#define DYLDARCH_I386    "i386"
#define DYLDARCH_X86_64  "x86_64"
#define DYLDARCH_ARMV6   "armv6"
#define DYLDARCH_ARMV7   "armv7"

typedef enum {
	kArmType,
	kIntelType
} cpu_type_t;

typedef enum {
	kArmv6,
	kArmv7,
	kIntelx86,
	kIntelx86_64
} cpu_subtype_t;

typedef struct architecture_t {
	char* name;
	endian_t cpu_endian;
	cpu_type_t cpu_type;
	cpu_subtype_t cpu_subtype;
} architecture_t;

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
	architecture_t* arch;
	dyldcache_header_t* header;
} dyldcache_t;

dyldcache_t* dyldcache_create();
dyldcache_t* dyldcache_open(const char* path);
void dyldcache_debug(dyldcache_t* cache);
void dyldcache_free(dyldcache_t* cache);

architecture_t* dyldcache_architecture_create();
architecture_t* dyldcache_architecture_parse(unsigned char* data);
void dyldcache_architecture_debug(architecture_t* arch);
void dyldcache_architecture_free(architecture_t* arch);

dyldcache_header_t* dyldcache_header_create();
dyldcache_header_t* dyldcache_header_parse(unsigned char* data);
void dyldcache_header_debug(dyldcache_header_t* header);
void dyldcache_header_free(dyldcache_header_t* header);

#endif /* DYLDCACHE_H_ */
