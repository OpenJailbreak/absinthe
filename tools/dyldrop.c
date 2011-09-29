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

#include "debug.h"
#include "common.h"
#include "dyldcache.h"

int main(int argc, char* argv[]) {
	int ret = 0;
	if(argc != 2) {
		info("Usage: ./dyldrop dyld_shared_cache_armv7\n");
		return 0;
	}
	char* dyldcache_path = strdup(argv[1]);

	debug("Creating dyldcache from %s\n", dyldcache_path);
	dyldcache_t* cache = dyldcache_open(dyldcache_path);
	if(cache == NULL) {
		error("Unable to allocate memory for dyldcache\n");
		goto panic;
	}

	goto finish;

panic:
	error("ERROR: %d\n", ret == 0 ? -1 : ret);

finish:
	debug("Cleaning up\n");
	if(cache) dyldcache_free(cache);
	return ret;
}
