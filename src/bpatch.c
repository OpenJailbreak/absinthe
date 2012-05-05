/**
  * GreenPois0n Apparition - bpatch.c
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

#include <bzlib.h>

#include "bpatch.h"

bpatch_t* bpatch_create() {
	bpatch_t* bpatch = (bpatch_t*) malloc(sizeof(bpatch_t));
	if(bpatch) {
		memset(bpatch, '\0', sizeof(bpatch_t));
	}
	return bpatch;
}

bpatch_t* bpatch_open(const char* path) {
	bpatch_t* bpatch = bpatch_create();
	return bpatch;
}

int bpatch_apply(bpatch_t* patch, const char* path) {
	return 0;
}

void bpatch_free(bpatch_t* bpatch) {
	if(bpatch) {
		free(bpatch);
	}
}
