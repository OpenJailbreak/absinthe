/*
 * bpatch.c
 *
 *  Created on: May 4, 2012
 *      Author: posixninja
 */

#include <stdio.h>
#include "bpatch.h"

int main(int argc, char* argv[]) {
	int err = 0; // Error to return
	bpatch_t* patch = NULL; // Handle to our patch file
	char* patch_path = NULL; // Path to the patch file
	char* target_path = NULL; // Path to the target file

	// Check for required arguments
	if(argc == 3) {
		target_path = strdup(argv[1]);
		patch_path = strdup(argv[2]);
	} else {
		printf("usage: ./binpatch <target> <patch>\n");
		return -1;
	}

	// Make file our path strings were cloned correctly
	if(target_path && patch_path) {

		// Open up handle to the patch
		patch = bpatch_open(patch_path);
		if(patch != NULL) {
			// Debugger
			bpatch_debug(patch);

			// Successfully opened path
			//  apply it to our target file
			if(bpatch_apply(patch, target_path) != 0) {
				printf("Failed to patch target\n");
				err = -1;
			}

			// We don't need this any longer
			bpatch_free(patch);

		} else {
			// Unable to open patch file
			//  is the path correct? is the format correct?
			printf("Unable to open patch file %s\n", patch_path);
			err = -1;
		}

		// We have no need for these any longer
		free(patch_path);
		free(target_path);

	} else {
		// WTF, we should never be here...
		err = -1;
	}

	return err;
}
