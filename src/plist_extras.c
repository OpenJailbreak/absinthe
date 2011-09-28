/**
  * GreenPois0n Apparition - plist_extras.c
  * Copyright (C) 2010 Chronic-Dev Team
  * Copyright (C) 2010 Nikias Bassen
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

#include "plist_extras.h"

static void buffer_write_to_filename(const char *filename, const char *buffer, uint64_t length)
{
	FILE *f;

	f = fopen(filename, "ab");
	if (!f)
		f = fopen(filename, "wb");
	if (f) {
		fwrite(buffer, sizeof(char), length, f);
		fclose(f);
	}
}

int plist_write_to_filename(plist_t plist, const char *filename, enum plist_format_t format)
{
	char *buffer = NULL;
	uint32_t length;

	if (!plist || !filename)
		return 0;

	if (format == PLIST_FORMAT_XML)
		plist_to_xml(plist, &buffer, &length);
	else if (format == PLIST_FORMAT_BINARY)
		plist_to_bin(plist, &buffer, &length);
	else
		return 0;

	buffer_write_to_filename(filename, buffer, length);

	free(buffer);

	return 1;
}
