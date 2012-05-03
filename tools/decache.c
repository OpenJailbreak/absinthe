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
	int err = 0;
	char* cache = NULL; // The path the dyldcache
	char* dylib = NULL; // The name of the dylib to extract
	dyldcache_t* dyldcache = NULL; // Handle to dyld cache
	dyldimage_t* dyldimage = NULL; // Handle to dyld image

	if(argc == 2) {
		// We need to free this when we're done with it
		cache = strdup(argv[1]);

	} else if(argc == 3) {
		// We need to free this when we're done with it
		dylib = strdup(argv[2]);

	} else {
		printf("usage: ./decache <dyldcache>\n");
		printf("       ./decache <dyldcache> <dylib>\n");
		return -1;
	}

	// Make sure cache was specified on the command line
	if(cache != NULL) {
		// Cache was specified on the command line
		//  so let's try openning it
		dyldcache_t* dyldcache = dyldcache_open(cache);
		if(dyldcache != NULL) {
			// Cache was successfully opened
			//  did they specify which dylib they wanted also?
			if(dylib != NULL) {
				// Dylib was specified on the command line
				//  so only extract it
				dyldimage = dyldcache_get_image(dyldcache, dylib);
				if(dyldimage != NULL) {
					// We've successfully found the dylib
					//  Let's write it to disk
					dyldimage_save(dyldimage, dylib);
					// dyldimage is freed when dyldcache is
					//  this might not be very safe if used incorrectly...

				} else {
					// Error locating the dylib in the provided cache
					//  are you sure the filename was correct?
					printf("Unable to find dylib in dyldcache\n");
					err = -1;
				}

				// Don't need the dylib string anymore
				free(dylib);

			} else {
				// No dylib was specified on the command line
				//  so extract all dylibs
				for(dyldimage = dyldcache_first_image(dyldcache);
					dyldimage != NULL;
					dyldimage = dyldcache_next_image(dyldcache, dyldimage)) {
						// Save each image
						dyldimage_save(dyldimage, dyldimage_get_name(dyldimage));
				}
			}

			// Don't need this the handle to dyldcache anymore
			//  This also frees dyldimage if it exists
			dyldcache_free(dyldcache);

		} else {
			// We were unable to open the dyldcache
			//  Is the path correct? Has the format changed?
			printf("Unable to open dyldcache\n");
			err = -1;
		}

		// We don't need the cache string anymore
		free(cache);
	}

	return err;
}
