/**
 * GreenPois0n Absinthe - device_compat.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "device_compat.h"
#include "fsgen.h"

/* Load addresses of libcopyfile.dylib per device/build for iOS 5.0.x. */
static struct dev_vmaddr devices_vmaddr_libcopyfile[] = {
	/* iOS 5.0 */
	{ "iPad1,1",   "9A334", 0x327b2000 },
	{ "iPhone2,1", "9A334", 0x33748000 },
	{ "iPhone3,1", "9A334", 0x355f4000 },
	{ "iPhone4,1", "9A334", 0x36f60000 },
	/* iOS 5.0.1 */
	{ "iPad1,1",   "9A405", 0x3012f000 },
	{ "iPad2,1",   "9A405", 0x37d2a000 },
	{ "iPad2,2",   "9A405", 0x37bfc000 },
	{ "iPad2,3",   "9A405", 0x36c23000 },
	{ "iPhone2,1", "9A405", 0x34a52000 },
	{ "iPhone3,1", "9A405", 0x30654000 },
	{ "iPhone3,3", "9A405", 0x323b1000 },
	{ "iPhone4,1", "9A405", 0x31f54000 },
	{ "iPhone4,1", "9A406", 0x31cfb000 },
	{ "iPod3,1",   "9A405", 0x35202000 },
	{ "iPod4,1",   "9A405", 0x30c29000 },
	{ 0, 0, 0 }
};

/* Hardware model / build combinations supported by the jailbreak. */
static compatibility_t compatible_devices[] = {
	{"N92AP", "9B176"},
	{"N90AP", "9B176"},
	{"N88AP", "9B176"},
	{"N18AP", "9B176"},
	{"K48AP", "9B176"},

	{"N92AP", "9B206"},
	{"N90AP", "9B206"},
	{"N88AP", "9B206"},
	{"N18AP", "9B206"},
	{"K48AP", "9B206"},

	{"N92AP", "9A405"},
	{"N90AP", "9A405"},
	{"N88AP", "9A405"},
	{"N18AP", "9A405"},
	{"K48AP", "9A405"},

	{"N92AP", "9A334"},
	{"N90AP", "9A334"},
	{"N88AP", "9A334"},
	{"N18AP", "9A334"},
	{"K48AP", "9A334"},

	{"N90AP", "8L1"},
	{"N88AP", "8L1"},
	{"N18AP", "8L1"},
	{"K48AP", "8L1"},

	{"N90AP", "8K2"},
	{"N88AP", "8K2"},
	{"N18AP", "8K2"},
	{"K48AP", "8K2"},

	{NULL, NULL}
};

uint32_t get_libcopyfile_vmaddr(const char *product, const char *build)
{
	int i = 0;
	uint32_t vmaddr = 0;
	while (devices_vmaddr_libcopyfile[i].product) {
		if (!strcmp(product, devices_vmaddr_libcopyfile[i].product)
				&& !strcmp(build, devices_vmaddr_libcopyfile[i].build)) {
			vmaddr = devices_vmaddr_libcopyfile[i].vmaddr;
			break;
		}
		i++;
	}
	return vmaddr;
}

int jb_device_is_supported(const char *product, const char *build)
{
	if ((strcmp(build, "9B176") == 0) || (strcmp(build, "9B206") == 0)
			|| (strcmp(build, "9B208") == 0) || (strcmp(build, "11A501") == 0)) {
		char testfn[512];
		struct stat fst;
		snprintf(testfn, sizeof(testfn),
				"data/%s/%s/rocky-racoon/boot.conf", build, product);
		return (stat(testfn, &fst) == 0);
	} else {
		uint32_t vmaddr = get_libcopyfile_vmaddr(product, build);
		return (vmaddr != 0);
	}
}

int jb_check_consistency(const char *product, const char *build)
{
	if (strcmp(build, "11A501") == 0)
		return 0;

	if ((strcmp(build, "9B176") == 0) || (strcmp(build, "9B206") == 0)
			|| (strcmp(build, "9B208") == 0)) {
		char testfn[512];
		struct stat fst;

		snprintf(testfn, sizeof(testfn),
				"data/%s/%s/rocky-racoon/boot.conf", build, product);
		if (stat(testfn, &fst) != 0) {
			fprintf(stderr, "ERROR: missing file '%s'\n", testfn);
			return -2;
		}

		snprintf(testfn, sizeof(testfn),
				"data/%s/%s/rocky-racoon/install.conf", build, product);
		if (stat(testfn, &fst) != 0) {
			fprintf(stderr, "ERROR: missing file '%s'\n", testfn);
			return -2;
		}

		int i = 0;
		for (i = 0; i < 256; i++) {
			snprintf(testfn, sizeof(testfn),
					"data/%s/%s/rocky-racoon/%02x", build, product, i);
			if (stat(testfn, &fst) != 0) {
				fprintf(stderr, "ERROR: missing file '%s'\n", testfn);
				return -2;
			}
		}
		return 0;
	} else {
		return fsgen_check_consistency(build, product);
	}
}

int verify_product(char *product, char *build)
{
	compatibility_t *curcompat = &compatible_devices[0];
	while ((curcompat) && (curcompat->product != NULL)) {
		if (!strcmp(curcompat->product, product)
				&& !strcmp(curcompat->build, build))
			return 0;
		curcompat++;
	}
	return 1;
}

int check_consistency(char *product, char *build)
{
	struct stat buf;
	char prodstr[32];

	/* Verify main directory exists */
	snprintf(prodstr, 32, "payload/%s_%s", build, product);

	if (stat(prodstr, &buf) == -1) {
		fprintf(stderr, "Failed to open directory \"payload/%s\"\n", prodstr);
	}
	/* Seems legit */
	return 0;
}
