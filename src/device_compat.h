/**
 * GreenPois0n Absinthe - device_compat.h
 * Copyright (C) 2012 Chronic-Dev Team
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

#ifndef DEVICE_COMPAT_H
#define DEVICE_COMPAT_H

#include <stdint.h>

/* Per-device libcopyfile.dylib load address entry used for iOS 5.0.x exploitation. */
struct dev_vmaddr {
	const char *product;
	const char *build;
	uint32_t vmaddr;
};

/* Per-device hardware model / build compatibility entry. */
typedef struct _compatibility {
	char *product;
	char *build;
} compatibility_t;

/* Return the libcopyfile.dylib load address for a given product/build pair,
 * or 0 if not found. */
uint32_t get_libcopyfile_vmaddr(const char *product, const char *build);

/* Return 1 if the device/build combination is supported, 0 otherwise. */
int jb_device_is_supported(const char *product, const char *build);

/* Verify that all required payload files for the given product/build exist.
 * Returns 0 on success, negative on error. */
int jb_check_consistency(const char *product, const char *build);

/* Legacy: verify a device by hardware model and build string.
 * Returns 0 if supported, non-zero otherwise. */
int verify_product(char *product, char *build);

/* Legacy: check that the payload directory exists for the given product/build. */
int check_consistency(char *product, char *build);

#endif /* DEVICE_COMPAT_H */
