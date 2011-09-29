/**
 * GreenPois0n Absinthe - template.c
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
#include "template.h"

/*
 * Mach-O Segment Functions
 */
template_t* template_create() {
	template_t* command = NULL;
	return command;
}

template_t* template_load(unsigned char* data, unsigned int offset) {
	template_t* command = template_create();
	return command;
}

void template_debug(template_t* command) {

}

void template_free(template_t* command) {

}

/*
 * Mach-O Segment Info Functions
 */
template_info_t* template_info_create() {
	template_info_t* info = NULL;
	return info;
}

template_info_t* template_info_load(unsigned char* data, unsigned int offset) {
	template_info_t* info = template_info_create();
	return info;
}

void template_info_debug(template_info_t* info) {

}

void template_info_free(template_info_t* info) {

}
