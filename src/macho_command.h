/**
 * GreenPois0n Absinthe - macho_command.h
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

#ifndef MACHO_COMMAND_H_
#define MACHO_COMMAND_H_

#include "common.h"

#define MACHO_CMD_SEGMENT 0x1
#define	LC_SYMTAB	0x2
#define	LC_UNIXTHREAD	0x5
#define LC_UUID		0x1b
#define LC_CODE_SIGNATURE 0x1d

typedef struct macho_command_info_t {
	uint32_t cmd;
	uint32_t cmdsize;
} macho_command_info_t;

typedef struct macho_command_t {
	uint32_t cmd;
	uint32_t size;
	uint32_t offset;
	unsigned char* data;
	macho_command_info_t* info;
} macho_command_t;

/*
 * Mach-O Command Functions
 */
macho_command_t* macho_command_create();
macho_command_t* macho_command_load(unsigned char* data, unsigned int offset);
void macho_command_debug(macho_command_t* command);
void macho_command_free(macho_command_t* command);

/*
 * Mach-O Command Info Functions
 */
macho_command_info_t* macho_command_info_create();
macho_command_info_t* macho_command_info_load(unsigned char* data, unsigned int offset);
void macho_command_info_debug(macho_command_info_t* info);
void macho_command_info_free(macho_command_info_t* info);

#endif /* MACHO_COMMAND_H_ */
