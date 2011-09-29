/**
  * GreenPois0n Absinthe - dyldmap.c
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

dyldmap_t* dyldmap_create() {
	dyldmap_t* map = (dyldmap_t*) malloc(sizeof(dyldmap_t));
	if(map) {
		memset(map, '\0', sizeof(dyldmap_t));
	}
	return map;
}

dyldmap_t* dyldmap_parse(unsigned char* data) {
	dyldmap_t* map = dyldmap_create();
	if(map) {
		// TODO: Parse image
	}
	return map;
}

void dyldmap_free(dyldmap_t* map) {
	if(map) {
		free(map);
	}
}


void dyldmap_debug(dyldmap_t* image) {
	printf("\tMap:\n");
	printf("\n");
}
