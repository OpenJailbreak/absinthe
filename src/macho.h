/**
 * GreenPois0n Absinthe - mb2.h
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

#ifndef MACHO_H_
#define MACHO_H_

typedef struct macho_header_t {
	uint32_t magic;
} macho_header_t;

typedef struct macho_t {
	macho_header_t* header;
} macho_t;

/*
 * Mach-O Functions
 */
macho_t* macho_create();
macho_t* macho_open(const char* path);
void macho_debug(macho_t* macho);
void macho_free(macho_t* macho);

/*
 * Mach-O Header Functions
 */
macho_header_t* macho_header_create();
macho_header_t* macho_header_load(unsigned char* data, unsigned int offset);
void macho_header_debug(macho_t* macho);
void macho_header_free(macho_t* macho);

#endif /* MACHO_H_ */
