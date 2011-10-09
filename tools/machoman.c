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

int main(int argc, char* argv[]) {
	if(argc != 2) {
		info("Usage: ./machoman <mach-o file>\n");
		return -1;
	}

	macho_t* macho = macho_open(argv[1]);
	if(macho == NULL) {
		error("Unable to open macho file\n");
	}

	macho_debug(macho);
	macho_free(macho);
	return 0;
}
