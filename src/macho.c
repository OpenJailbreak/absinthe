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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "common.h"
#include "macho.h"

/*
 * Mach-O Functions
 */
macho_t* macho_create() {
	macho_t* macho = NULL;
	return macho;
}

macho_t* macho_open(const char* path) {
	macho_t* macho = NULL;
	return macho;
}

void macho_debug(macho_t* macho) {

}

void macho_free(macho_t* macho) {

}

/*
 * Mach-O Header Functions
 */
macho_header_t* macho_header_create() {
	macho_header_t* header = NULL;
	return header;
}

macho_header_t* macho_header_load(unsigned char* data, unsigned int offset) {
	macho_header_t* header = macho_header_create();
	return header;
}

void macho_header_debug(macho_header_t* header) {

}

void macho_header_free(macho_header_t* header) {

}

/*
 * Mach-O Segments Functions
 */
macho_segment_t** macho_segments_create(uint32_t count) {
	macho_segment_t** segments = NULL;
	return segments;
}

macho_segment_t** macho_segments_load(macho_t* macho) {
	macho_segment_t** segments = macho_segments_create(0);
	return segments;
}

void macho_segments_debug(macho_segment_t** segments) {
	debug("\tSegments:\n");
	debug("\n");
}

void macho_segments_free(macho_segment_t** segments) {
	// TODO: Loop through and free each item
	if(segments) {
		free(segments);
	}
}

/*
 * Mach-O Sections Functions
 */
macho_section_t** macho_sections_create(uint32_t count) {
	macho_segment_t** sections = NULL;
	return sections;
}

macho_section_t** macho_sections_load(macho_t* macho) {
	macho_section_t** sections = macho_sections_create(0);
	return sections;
}

void macho_sections_debug(macho_section_t** sections) {
	debug("\tSections:\n");
	debug("\n");
}

void macho_sections_free(macho_section_t** sections) {
	// TODO: Loop through and free each item
	if(sections) {
		free(sections);
	}
}

/*
 * Mach-O Commands Functions
 */
macho_command_t** macho_commands_create(uint32_t count) {
	macho_segment_t** commands = NULL;
	return commands;
}

macho_command_t** macho_commands_load(macho_t* macho) {
	macho_command_t** commands = macho_commands_create(0);
	return commands;
}

void macho_commands_debug(macho_command_t** commands) {
	debug("\tCommands:\n");
	debug("\n");
}

void macho_commands_free(macho_command_t** commands) {
	if(commands) {
		free(commands);
	}
}
