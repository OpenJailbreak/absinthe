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

#include "file.h"
#include "debug.h"
#include "common.h"
#include "macho.h"

/*
 * Mach-O Functions
 */
macho_t* macho_create() {
	macho_t* macho = (macho_t*) malloc(sizeof(macho_t));
	if (macho) {
		memset(macho, '\0', sizeof(macho));
	}
	return macho;
}

macho_t* macho_open(const char* path) {
	int i = 0;
	int err = 0;
	macho_t* macho = NULL;
	unsigned int size = 0;
	unsigned char* data = NULL;

	macho = macho_create();
	if (macho) {
		macho->file = file_create();
		if (macho->file == NULL) {
			error("Unable to create file object\n");
			return NULL;
		}

		debug("Reading Mach-O file from path\n");
		err = file_read(path, &data, &size);
		if (err < 0) {
			error("Unable to read Mach-O file\n");
			macho_free(macho);
			return NULL;
		}
		macho->file->data = data;
		macho->file->size = size;
		macho->file->path = path;

		debug("Copying path into Mach-O object\n");
		macho->path = strdup(path);
		if (macho->path == NULL) {
			error("Unable to copy Mach-O path\n");
			macho_free(macho);
			return NULL;
		}

		debug("Loading Mach-O header\n");
		macho->header = macho_header_load(data, size);
		if (macho->header == NULL) {
			error("Unable to load Mach-O header information\n");
			macho_free(macho);
			return NULL;
		}

		debug("Loading Mach-O command\n");
		macho->commands = macho_commands_load(macho);
		if (macho->commands == NULL) {
			error("Unable to parse Mach-O load commands\n");
			macho_free(macho);
			return NULL;
		}

		debug("Handling Mach-O commands\n");
		for (i = 0; i < macho->command_count; i++) {
			macho_handle_command(macho, macho->commands[i]);
		}
		// TODO: Remove the line below this once debugging is finished
		macho_debug(macho);
	}
	return macho;
}

void macho_debug(macho_t* macho) {
	if (macho) {
		debug("Mach-O:\n");

		if (macho->header)
			macho_header_debug(macho->header);

		if (macho->commands)
			macho_commands_debug(macho->commands);

		if (macho->segments)
			macho_segments_debug(macho->segments);

		debug("\n");
	}
}

void macho_free(macho_t* macho) {
	if (macho) {
		if (macho->header) {
			macho_header_free(macho->header);
			macho->header = NULL;
		}
		if (macho->commands) {
			macho_commands_free(macho->commands);
			macho->commands = NULL;
		}
		if (macho->segments) {
			macho_segments_free(macho->segments);
			macho->segments = NULL;
		}
		if (macho->path) {
			free(macho->path);
			macho->path = NULL;

		}
		free(macho);
	}
}

/*
 * Mach-O Header Functions
 */
macho_header_t* macho_header_create() {
	macho_header_t* header = (macho_header_t*) malloc(sizeof(macho_header_t));
	if (header) {
		memset(header, '\0', sizeof(macho_header_t));
	}
	return header;
}

macho_header_t* macho_header_load(unsigned char* data, unsigned int offset) {
	macho_header_t* header = macho_header_create();
	if (header) {
		memcpy(header, &data[offset], sizeof(macho_header_t));
	}
	return header;
}

void macho_header_debug(macho_header_t* header) {
	if (header) {
		debug("\tHeader:\n");
		debug("\t\t     magic = 0x%08x\n", header->magic);
		debug("\t\t   cputype = 0x%08x\n", header->cputype);
		debug("\t\tcpusubtype = 0x%08x\n", header->cpusubtype);
		debug("\t\t  filetype = 0x%08x\n", header->filetype);
		debug("\t\t     ncmds = 0x%08x\n", header->ncmds);
		debug("\t\tsizeofcmds = 0x%08x\n", header->sizeofcmds);
		debug("\t\t     flags = 0x%08x\n", header->flags);
		debug("\t\n");
	}
}

void macho_header_free(macho_header_t* header) {
	if (header) {
		free(header);
	}
}

int macho_handle_command(macho_t* macho, macho_command_t* command) {
	if(macho) {
		// If load command is a segment command, then load a segment
		//  if a symbol table, then load a symbol table... etc...
		switch(command->info->magic) {
		default:
			return -1;
		}
	}
	return 0;
}

/*
 * Mach-O Commands Functions
 */
macho_command_t** macho_commands_create(uint32_t count) {
	uint32_t size = count * sizeof(macho_command_t*);
	macho_command_t** commands = (macho_command_t**) malloc(size + 1);
	if (commands) {
		memset(commands, '\0', size+1);
	}
	return commands;
}

macho_command_t** macho_commands_load(macho_t* macho) {
	int i = 0;
	uint32_t count = 0;
	macho_command_t** commands = NULL;
	if (macho) {
		count = macho->command_count;
		commands = macho_commands_create(count);
		if (commands) {
			for (i = 0; i < count; i++) {
				commands[i] = macho_command_load(macho->file->data, macho->file->offset);
				if (commands[i] == NULL) {
					error("Unable to parse Mach-O load command\n");
					macho_commands_free(commands);
					return NULL;
				}
			}
		}
	}
	return commands;
}

void macho_commands_debug(macho_command_t** commands) {
	int i = 0;
	if (commands) {
		debug("\tCommands:\n");
		while (commands[i] != NULL) {
			macho_command_debug(commands[i++]);
		}
		debug("\t\n");
	}
}

void macho_commands_free(macho_command_t** commands) {
	int i = 0;
	if (commands) {
		while (commands[i] != NULL) {
			macho_command_free(commands[i]);
			commands[i] = NULL;
			i++;
		}
		free(commands);
	}
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
	if (segments) {
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
	debug("\t\n");
}

void macho_sections_free(macho_section_t** sections) {
	// TODO: Loop through and free each item
	if (sections) {
		free(sections);
	}
}
