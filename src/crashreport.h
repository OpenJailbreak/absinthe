/**
 * GreenPois0n Absinthe - crashreport.h
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

#ifndef CRASHREPORT_H_
#define CRASHREPORT_H_

#include <plist/plist.h>

 typedef struct dylib_info {
	char* name;
	uint32_t offset;
} dylib_info_t;

typedef struct arm_state_t {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t ip;
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
	uint32_t cpsr;
} arm_state_t;

/* The actual crashreport object containing it's data */
typedef struct crashreport_t {
	char *name;
	unsigned int pid;
	arm_state_t* state;
	dylib_info_t** dylibs;
} crashreport_t;

crashreport_t* crashreport_create();
void crashreport_free(crashreport_t* report);
void crashreport_debug(crashreport_t* report);

crashreport_t* crashreport_parse_plist(plist_t crash);
char* crashreport_parse_name(const char* description);
unsigned int crashreport_parse_pid(const char* description);
arm_state_t* crashreport_parse_state(const char* description);
dylib_info_t** crashreport_parse_dylibs(const char* description);

#endif /* CRASHREPORT_H_ */
