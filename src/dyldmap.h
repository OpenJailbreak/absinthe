/**
  * GreenPois0n Absinthe - dyldmap.h
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

#ifndef DYLDMAP_H_
#define DYLDMAP_H_

#include "common.h"

typedef struct dyldmap_info_t {
	uint64_t address;
	uint64_t size;
	uint64_t offset;
	uint32_t maxProt;
	uint32_t initProt;
} dyldmap_info_t;

typedef struct dyldmap_t {
	dyldmap_info_t* info;
} dyldmap_t;

dyldmap_t* dyldmap_create();
dyldmap_t* dyldmap_parse(unsigned char* data);
void dyldmap_debug(dyldmap_t* image);
void dyldmap_free(dyldmap_t* map);


#endif /* DYLDMAP_H_ */
