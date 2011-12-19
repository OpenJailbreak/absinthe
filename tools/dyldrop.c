/**
 * GreenPois0n Absinthe - dyldrop.c
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

#include "macho.h"
#include "debug.h"
#include "common.h"
#include "dyldcache.h"

int main(int argc, char* argv[]) {
	int i = 0;
	int ret = 0;
	char* path = NULL;
	char* dylib = NULL;
	char* symbol = NULL;
	void* address = NULL;
	macho_t* macho = NULL;
	dyldimage_t* image = NULL;
	dyldcache_t* cache = NULL;

	if (argc != 4) {
		info("Usage: ./dyldrop <dyldcache> <dylib> <symbol>\n");
		return 0;
	}
	path = strdup(argv[1]);
	dylib = strdup(argv[2]);
	symbol = strdup(argv[3]);

	debug("Creating dyldcache from %s\n", path);
	cache = dyldcache_open(path);
	if (cache == NULL) {
		error("Unable to allocate memory for dyldcache\n");
		goto panic;
	}

	for (i = 0; i < cache->header->images_count; i++) {
		image = cache->images[i];
		debug("Found %s\n", image->name);
		if (strcmp(dylib, image->name) == 0) {
			macho = macho_load(image->data, image->size);
			if (macho == NULL) {
				debug("Unable to parse Mach-O file in cache\n");
				continue;
			}
			//macho_debug(dylib);

			address = macho_lookup(macho, symbol);
			if (address != NULL) {
				printf("#define %s (void*)0x%08x\n", symbol, address);
			}

			//macho_free(dylib);
			//macho = NULL;
		}
	}
	if(address = NULL) {

	}

	dyldcache_free(cache);
	cache = NULL;
	goto finish;

	panic:
	error("ERROR: %d\n", ret == 0 ? -1 : ret);

	finish: debug("Cleaning up\n");
	if (cache)
		dyldcache_free(cache);
	if (macho)
		macho_free(macho);
	if (path)
		free(path);
	return ret;
}
