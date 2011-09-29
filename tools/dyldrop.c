/*
 ============================================================================
 Name        : dyldrop.c
 Author      : Joshua Hill
 Version     :
 Copyright   : © 2011 Chronic-Dev, LLC
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dyldcache.h"

int main(int argc, char* argv[]) {
	int ret = 0;
	if(argc != 2) {
		fprintf(stderr, "Usage: ./dyldrop dyld_shared_cache_armv7\n");
		return 0;
	}
	char* dyldcache_path = strdup(argv[1]);

	fprintf(stderr, "Creating dyldcache from %s\n", dyldcache_path);
	dyldcache_t* cache = dyldcache_open(dyldcache_path);
	if(cache == NULL) {
		fprintf(stderr, "Unable to allocate memory for dyldcache\n");
		goto panic;
	}

	goto finish;

panic:
	fprintf(stderr, "ERROR: %d\n", ret == 0 ? -1 : ret);

finish:
	fprintf(stderr, "Cleaning up\n");
	if(cache) dyldcache_free(cache);
	return ret;
}
