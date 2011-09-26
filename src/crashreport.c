/**
 * GreenPois0n Absinthe - crashreport.c
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

#include <plist/plist.h>
#include <libimobiledevice/libimobiledevice.h>

#include "debug.h"
#include "crashreport.h"

crashreport_t* crashreport_create() {
	crashreport_t* report = (crashreport_t*) malloc(sizeof(crashreport_t));
	if (report) {
		memset(report, '\0', sizeof(crashreport_t));
	}
	return report;
}

void crashreport_free(crashreport_t* report) {
	if (report) {
		free(report);
	}
}

arm_state_t* crashreport_parse_state(const char* description) {
	return NULL;
}

dylib_info_t** crashreport_parse_dylibs(const char* description) {
	return NULL;
}

thread_info_t** crashreport_parse_threads(const char* description) {
	return NULL;
}

crashreport_t* crashreport_parse_plist(plist_t plist) {
	char* description = NULL;
	plist_t description_node = NULL;
	crashreport_t* crashreport = NULL;

	// The description element is the one with all the good stuff
	description_node = plist_dict_get_item(plist, "description");
	if (description_node && plist_get_node_type(description_node) == PLIST_STRING) {
		plist_get_string_val(description_node, &description);

		crashreport = crashreport_create();
		if(crashreport == NULL) {
			error("Unable to allocate memory for crashreport\n");
			return NULL;
		}

		crashreport->state = crashreport_parse_state(description);
		if(crashreport->state == NULL) {
			error("Unable to parse ARM state from crashreport\n");
			crashreport_free(crashreport);
			return NULL;
		}

		crashreport->dylibs = crashreport_parse_dylibs(description);
		if(crashreport->dylibs == NULL) {
			error("Unable to parse dylib base addresses from crashreport\n");
			crashreport_free(crashreport);
			return NULL;
		}

		crashreport->threads = crashreport_parse_threads(description);
		if(crashreport->threads == NULL) {
			error("Unable to parse thread info from crashreport\n");
			crashreport_free(crashreport);
			return NULL;
		}
	}

	return crashreport;
}
