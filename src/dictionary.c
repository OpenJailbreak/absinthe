/**
  * GreenPois0n Absinthe - dictionary.c
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

	int size = 0x40;
	unsigned char* buffer = malloc(size);
	memset(buffer, '\0', size);
	dictionary_t* ht = (dictionary_t*) buffer;

	uint32_t flags = DICTIONARY_LINEAR_HASHING | DICTIONARY_HAS_KEYS | DICTIONARY_HAS_COUNTS;
	//flags |= kCFBasicHashAggressiveGrowth | kCFBasicHashHasKeys |  | kCFBasicHashHasHashCache;
	//ht->_cfisa = 0x3fdec0c0;
	//ht->_cfinfo = 0x0100078c;
	ht->bits.finalized = 0;
	ht->bits.strong_values = (flags & DICTIONARY_STRONG_VALUES) ? 1 : 0;
    ht->bits.strong_values2 = 0;
    ht->bits.strong_keys = (flags & DICTIONARY_STRONG_KEYS) ? 1 : 0;
    ht->bits.strong_keys2 = 0;
    ht->bits.hash_style = (flags >> 13) & 0x3;
    ht->bits.fast_grow = (flags & DICTIONARY_AGGRESSIVE_GROWTH) ? 1 : 0;
    ht->bits.__0 = 0;
    ht->bits.num_buckets_idx = 0;
    ht->bits.used_buckets = 1028;
    ht->bits.marker = 0;
    ht->bits.deleted = 0;
    ht->bits.mutations = 2;

	uint64_t offset = 1;
    ht->bits.values2_offset = 0;
    ht->bits.keys_offset = (flags & DICTIONARY_HAS_KEYS) ? 1 : 0;
    ht->bits.keys2_offset = 0;
    ht->bits.counts_offset = (flags & DICTIONARY_HAS_COUNTS) ? 1 : 0;
    ht->bits.hashes_offset = (flags & DICTIONARY_HAS_HASHCACHE) ? 1 : 0;


    ht->callbacks = 0x36010100;
    ht->pointers[0] = 0x37010100;
    ht->pointers[1] = 0x37010100;
    ht->pointers[2] = 0x37010100;

    // overrides
    //buffer[11] = 0x01;
    //memset(buffer, 'A', 0x20);

	printf("ht->bits.hash_style = %d\n", ht->bits.hash_style);
    printf("ht->bits.values2_offset = %d\n", ht->bits.values2_offset);
    printf("ht->bits.keys_offset = %d\n", ht->bits.keys_offset);
    printf("ht->bits.keys2_offset = %d\n", ht->bits.keys2_offset);
    printf("ht->bits.counts_offset = %d\n", ht->bits.counts_offset);
    printf("ht->bits.orders_offset = %d\n", ht->bits.orders_offset);
    printf("ht->bits.hashes_offset = %d\n", ht->bits.hashes_offset);
    printf("ht->bits.num_buckets_idx = %d\n", ht->bits.num_buckets_idx);
    printf("ht->bits.used_buckets = %d\n", ht->bits.used_buckets);
    printf("ht->bits.finalized = %d\n", ht->bits.finalized);
    printf("ht->bits.fast_grow = %d\n", ht->bits.fast_grow);
    printf("ht->bits.strong_values = %d\n", ht->bits.strong_values);
    printf("ht->bits.strong_values2 = %d\n", ht->bits.strong_values2);
    printf("ht->bits.strong_keys = %d\n", ht->bits.strong_keys);
    printf("ht->bits.strong_keys2 = %d\n", ht->bits.strong_keys2);
    printf("ht->bits.marker = %d\n", ht->bits.marker);
    printf("ht->bits.deleted = %d\n", ht->bits.deleted);
    printf("ht->bits.mutations = %d\n", ht->bits.mutations);
printf("\n");

	int i = 0;
	int j = 1;
	for(i = 0; i < size; i++) {
		if((i % 0x10) == 0 && i != 0) printf("\n");
			if(buffer[i] == 0) {
				buffer[i] = j++;
			}
			if(buffer[i] >= 0x70) {
				buffer[i] = 0x50;
			}
		printf("\\x%02x", buffer[i]);
	}
	printf("\n");
	printf("ht->bits.hash_style = %d\n", ht->bits.hash_style);
    printf("ht->bits.values2_offset = %d\n", ht->bits.values2_offset);
    printf("ht->bits.keys_offset = %d\n", ht->bits.keys_offset);
    printf("ht->bits.keys2_offset = %d\n", ht->bits.keys2_offset);
    printf("ht->bits.counts_offset = %d\n", ht->bits.counts_offset);
    printf("ht->bits.orders_offset = %d\n", ht->bits.orders_offset);
    printf("ht->bits.hashes_offset = %d\n", ht->bits.hashes_offset);
    printf("ht->bits.num_buckets_idx = %d\n", ht->bits.num_buckets_idx);
    printf("ht->bits.used_buckets = %d\n", ht->bits.used_buckets);
    printf("ht->bits.finalized = %d\n", ht->bits.finalized);
    printf("ht->bits.fast_grow = %d\n", ht->bits.fast_grow);
    printf("ht->bits.strong_values = %d\n", ht->bits.strong_values);
    printf("ht->bits.strong_values2 = %d\n", ht->bits.strong_values2);
    printf("ht->bits.strong_keys = %d\n", ht->bits.strong_keys);
    printf("ht->bits.strong_keys2 = %d\n", ht->bits.strong_keys2);
    printf("ht->bits.marker = %d\n", ht->bits.marker);
    printf("ht->bits.deleted = %d\n", ht->bits.deleted);
    printf("ht->bits.mutations = %d\n", ht->bits.mutations);
	for(i = 0; i < size; i+=4) {
		if((i % 0x10) == 0 && i != 0) printf("\n");
		printf(" 0x%08x", *((unsigned int*) &buffer[i]));
	}

	buffer[size] = 0;
	*length = size;
	*data = buffer;
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
