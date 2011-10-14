/**
  * GreenPois0n Apparition - plist_extras.h
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

#ifndef PLIST_EXTRAS_H
#define PLIST_EXTRAS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <plist/plist.h>

enum plist_format_t {
	PLIST_FORMAT_XML,
	PLIST_FORMAT_BINARY
};

int plist_read_from_filename(plist_t *plist, const char *filename);
int plist_write_to_filename(plist_t plist, const char *filename, enum plist_format_t format);

#endif /* PLIST_EXTRAS_H */
