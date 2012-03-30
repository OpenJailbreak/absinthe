/**
 * GreenPois0n Gizmo - machoman.c
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

#include "macho.h"
#include "debug.h"
#include "common.h"

enum {
	OP_NONE,
	OP_INFO,
	OP_VIRT,
	OP_SEARCH
} op_mode_t;

static void print_usage(int argc, char **argv)
{
	char *name = NULL;
	
	name = strrchr(argv[0], '/');
	printf("Usage: %s <mach-o file> [OPTIONS] [PARAMS ...]\n", (name ? name + 1: argv[0]));
	printf("  -a|--address OFFSET\tget virtual address for given file offset.\n");
	printf("  -s|--search STRING\tsearch for STRING and print function addresses\n\t\tcontaining references to this string.\n");
	printf("\n");
}

static uint32_t get_virtual_address(macho_t* macho, uint32_t offset)
{
	if (macho->segment_count == 0) {
		error("no segments?\n");
		return 0;
	}

	uint32_t vaddr = 0;
	int i;

	for (i = 0; i < macho->segment_count; i++) {
		macho_segment_t* seg = macho->segments[i];
		if ((offset >= seg->offset) && (offset < seg->offset + seg->size)) {
			vaddr = (offset + seg->address) - seg->offset;
			break;
		}
	}
	return vaddr;
}

int main(int argc, char* argv[])
{
	uint32_t offset = 0;
	char* search = NULL;
	int search_len = 0;
	int mode = (argc < 2) ? OP_NONE : OP_INFO;
	int i;

	/* parse cmdline args */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--address")) {
			i++;
			if (!argv[i]) {
				print_usage(argc, argv);
				return 0;
			}
			sscanf(argv[i], "%i", &offset);
			mode = OP_VIRT;
			continue;
		}
		else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--search")) {
			i++;
			if (!argv[i]) {
				print_usage(argc, argv);
				return 0;
			}
			search = strdup(argv[i]);
			search_len = strlen(search);
			mode = OP_SEARCH;
			continue;
		}
	}

	if (mode == OP_NONE) {
		print_usage(argc, argv);
		return 0;
	}

	macho_t* macho = macho_open(argv[1]);
	if(macho == NULL) {
		error("Unable to open macho file\n");
	}

	switch (mode) {
	case OP_VIRT:
		{
			uint32_t vaddr = get_virtual_address(macho, offset);
			if (vaddr > 0) {
				printf("0x%08x\n", vaddr);
			} else {
				printf("Not found...\n");
			}
		}
		break;
	case OP_SEARCH:
		{
		int found = 0;
		for (i = 0; i < macho->size; i++) {
			if (macho->data[i] != search[0]) {
				continue;
			}
			if (memcmp(macho->data + i, search, search_len) != 0) {
				continue;
			}

			// found match. go back to the beginning of the string
			offset = i;
			uint32_t saddr;
			found++;

			while (offset > 0 && (macho->data[offset-1] != '\0')) {
				offset--;
			}
			debug("Found match in string '%s', offset 0x%08x\n", macho->data + offset, offset);
			saddr = get_virtual_address(macho, offset);
			if (saddr == 0) {
				error("Error: could not get virtual address for offset 0x%08x\n", offset);
				continue;
			}
			debug("Virtual address: 0x%08x\n", saddr);
			int j;
			for (j = 0; j < macho->size; j+=4) {
				if (*(uint32_t*)(macho->data+j) == saddr) {
					uint32_t vaddr = get_virtual_address(macho, j);
					debug("found reference at offset 0x%08x, vaddr=0x%08x\n", j, vaddr);
					offset = j;
					while (offset > 0 && ((*(uint16_t*)(macho->data+offset) & 0xFF0F) != 0xB500)) {
						offset -= 2;
					}
					debug("found push instruction at offset 0x%08x\n", offset);
					printf("function 0x%08x\n", get_virtual_address(macho, offset));
				}
			}
		}
		if (!found) {
			printf("string '%s' not found!\n", search);
		}
		}
		break;
	case OP_INFO:
		macho_debug(macho);
		break;
	default:
		printf("invalid mode?!\n");
	}

leave:
	macho_free(macho);
	return 0;
}
