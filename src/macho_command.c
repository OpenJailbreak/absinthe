/**
 * GreenPois0n Absinthe - macho_command.c
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
#include "macho_command.h"

/*
 * Mach-O Command Functions
 */
macho_command_t* macho_command_create() {
	macho_command_t* command = (macho_command_t*) malloc(sizeof(macho_command_t));
	if (command) {
		memset(command, '\0', sizeof(macho_command_t));
	}
	return command;
}

macho_command_t* macho_command_load(unsigned char* data, unsigned int offset) {
	unsigned int size = 0;
	macho_command_t* command = macho_command_create();
	if (command) {
		command->info = macho_command_info_load(data, offset);
		if (command->info == NULL) {
			error("Unable to load Mach-O command info\n");
			macho_command_free(command);
			return NULL;
		}
		macho_command_debug(command);
	}
	return command;
}

void macho_command_debug(macho_command_t* command) {
	if (command) {
		debug("\tCommand:\n");
		if(command->info) {
			macho_command_info_debug(command->info);
		}
		debug("\t\n");
	}
}

void macho_command_free(macho_command_t* command) {
	if(command) {
		if(command->info) {
			macho_command_info_free(command->info);
			command->info = NULL;
		}
		free(command);
	}
}

/*
 * Mach-O Command Info Functions
 */
macho_command_info_t* macho_command_info_create() {
	macho_command_info_t* info = (macho_command_info_t*) malloc(sizeof(macho_command_info_t));
	if (info) {
		memset(info, '\0', sizeof(macho_command_info_t));
	}
	return info;
}

macho_command_info_t* macho_command_info_load(unsigned char* data, unsigned int offset) {
	macho_command_info_t* info = macho_command_info_create();
	if (info) {
		//debug("Command Offset = 0x%x\n", offset);
		memcpy(info, &data[offset], sizeof(macho_command_info_t*));
	}
	macho_command_info_debug(info);
	return info;
}

void macho_command_info_debug(macho_command_info_t* info) {
	if (info) {
		debug("\tInfo:\n");
		debug("\t\t    cmd = %d\n", info->cmd);
		debug("\t\tcmdsize = %d\n", info->cmd);
		debug("\t\n");
	}
}

void macho_command_info_free(macho_command_info_t* info) {
	if (info) {
		free(info);
	}
}
