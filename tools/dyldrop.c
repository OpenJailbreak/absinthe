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
	int ret = 0;
	if(argc != 3) {
		info("Usage: ./dyldrop <dyldcache> <kernel>\n");
		return 0;
	}
	char* cache_path = strdup(argv[1]);
	char* macho_path = strdup(argv[2]);
/*
	debug("Creating dyldcache from %s\n", cache_path);
	dyldcache_t* cache = dyldcache_open(cache_path);
	if(cache == NULL) {
		error("Unable to allocate memory for dyldcache\n");
		goto panic;
	}
	//dyldcache_free(cache);
*/
	debug("Creating macho from %s\n", macho_path);
	macho_t* macho = macho_open(macho_path);
	if(macho == NULL) {
		error("Unable to allocate memory for macho\n");
		goto panic;
	}
	//macho_free(macho);

	goto finish;

panic:
	error("ERROR: %d\n", ret == 0 ? -1 : ret);

finish:
	debug("Cleaning up\n");
	if(macho) macho_free(macho);
	//if(cache) dyldcache_free(cache);
	if(macho_path) free(macho_path);
	if(cache_path) free(cache_path);
	return ret;
}
