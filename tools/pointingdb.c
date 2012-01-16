/**
  * GreenPois0n Absinthe - pointingdb.c
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

#include "file.h"
#include "debug.h"
#include "common.h"

/*
 * Small program I wrote to help me find usable addresses in iPhone memory
 */

int main(int argc, char* argv) {
	int i = 0;
	unsigned int size = 0;
	unsigned char* data = NULL;

	uint32_t value = 0;
	uint32_t address = 0;
	uint32_t base = (uint32_t) 0x3F300000;

	//debug("Reading in pointer database\n");
	file_read("target.mem", &data, &size);
	if(data && size) {
		//debug("Database read, reconstructing addresses and filtering ASCII\n");
		for(i = 0; i < size; i+=4) {
			value = *((uint32_t*) &data[i]);
			if((value == 0) || ((value & 0xFF000000) == 0)) continue;
			address = base + i;
			//debug("Read pointer 0x%08x at offset 0x%x\n", address, i);
			//debug("Checking if string is ASCII safe\n");
			if(check_ascii_pointer(address)) {
				//debug("Safe ASCII pointer contains 0x%08x\n", value);
				debug("0x%08x, 0x%08x\n", address, value);
				//printf("print 0x%08x\n", value);
				//printf("print *+$\n");
				//printf("print *+$\n");
				//file_append("pointer.db", &address, 4);
			} else {
				//error("Error!! This pointer will not get past the ASCII filter\n");
			}
		}
	} else {
		error("Unable to read in file, returned %p and %d\n", data, size);
	}
	return 0;
}
