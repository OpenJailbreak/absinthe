/*
 * decache.c
 *
 *  Created on: Apr 30, 2012
 *      Author: posixninja
 */

#include <stdio.h>

#include <dyldcache.h>
#include <dyldimage.h>

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("usage: ./decache <dyldcache> <dylib>\n");
		return -1;
	}
	char* cache = strdup(argv[1]);
	char* dylib = strdup(argv[2]);
	if(cache == NULL || dylib == NULL) {
		printf("Invalid arguments\n");
		goto FINISH;
	}

	dyldcache_t* dyldcache = dyldcache_open(cache);
	if(dyldcache == NULL) {
		printf("Unable to open dyldcache\n");
		goto FINISH;
	}

	dyldimage_t* dyldimage = dyldcache_get_image(dyldcache, dylib);
	if(dyldimage == NULL) {
		printf("Unable to find dylib in dyldcache\n");
		goto FINISH;
	}
	dyldimage_save(dyldimage, dylib);

	FINISH:
	if(dyldcache) dyldcache_free(dyldcache);
	if(cache) free(cache);
	if(dylib) free(dylib);
	return 0;
}
