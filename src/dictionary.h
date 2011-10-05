/**
 * GreenPois0n Absinthe - dictionary.h
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
 * along with this program.  If not, see <dicttp://www.gnu.org/licenses/>.
 **/

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <stdint.h>

#define DICTIONARY_HASH_LINEAR        1
#define DICTIONARY_HASH_DOUBLE        2
#define DICTIONARY_HASH_EXPONENTIAL   3

#define DICTIONARY_HAS_VALUES2           (1UL << 2)
#define DICTIONARY_HAS_KEYS              (1UL << 3)
#define DICTIONARY_HAS_KEYS2             (1UL << 4)
#define DICTIONARY_HAS_COUNTS            (1UL << 5)
#define DICTIONARY_HAS_ORDER             (1UL << 6)
#define DICTIONARY_HAS_HASHCACHE         (1UL << 7)
#define DICTIONARY_STRONG_VALUES         (1UL << 9)
#define DICTIONARY_STRONG_VALUES2        (1UL << 10)
#define DICTIONARY_STRONG_KEYS           (1UL << 11)
#define DICTIONARY_STRONG_KEYS2          (1UL << 12)
#define DICTIONARY_LINEAR_HASHING        (DICTIONARY_HASH_LINEAR << 13)
#define DICTIONARY_DOUBLE_HASHING        (DICTIONARY_HASH_DOUBLE << 13)
#define DICTIONARY_EXPONENTIAL_HASHING   (DICTIONARY_HASH_EXPONENTIAL << 13)
#define DICTIONARY_AGGRESSIVE_GROWTH     (1UL << 15)

typedef union {
    uint32_t weak;
    uint32_t strong;
} dictionary_value_t;

typedef struct {
    uint32_t idx;
    uint32_t weak_key;
    uint32_t weak_key2;
    uint32_t weak_value;
    uint32_t weak_value2;
    uint32_t count;
    uint32_t order;
} dictionary_bucket_t;

typedef struct  {
    struct { // 128 bits
        uint64_t hash_style:2;
        uint64_t values2_offset:1;
        uint64_t keys_offset:2;
        uint64_t keys2_offset:2;
        uint64_t counts_offset:3;
        uint64_t orders_offset:3;
        uint64_t hashes_offset:3;
        uint64_t num_buckets_idx:6;
        uint64_t used_buckets:42;
        uint64_t __0:2;
        uint64_t finalized:1;
        uint64_t fast_grow:1;
        uint64_t strong_values:1;
        uint64_t strong_values2:1;
        uint64_t strong_keys:1;
        uint64_t strong_keys2:1;
        uint64_t marker:24;
        uint64_t deleted:16;
        uint64_t mutations:16;
    } bits;
    uint32_t callbacks;
    uint32_t pointers[4];
} __attribute__((__packed__)) dictionary_t;

dictionary_t* dictionary_create();
void dictionary_free(dictionary_t* dict);
void dictionary_debug(dictionary_t* dict);

int dictionary_make_attack(uint32_t address, uint32_t salt, char** data, int* length);

dictionary_value_t* dictionary_get_values(dictionary_t* dict);
void dictionary_set_values(dictionary_t* dict, dictionary_value_t *ptr);

dictionary_value_t* dictionary_get_values2(dictionary_t* dict);
void dictionary_set_values2(dictionary_t* dict, dictionary_value_t *ptr);

dictionary_value_t* dictionary_get_keys(dictionary_t* dict);
void dictionary_set_keys(dictionary_t* dict, dictionary_value_t *ptr);

dictionary_value_t* dictionary_get_keys2(dictionary_t* dict);
void dictionary_set_keys2(dictionary_t* dict, dictionary_value_t *ptr);

uintptr_t* dictionary_get_counts(dictionary_t* dict);
void dictionary_set_counts(dictionary_t* dict, uintptr_t *ptr);

uintptr_t* dictionary_get_orders(dictionary_t* dict);
void dictionary_set_orders(dictionary_t* dict, uintptr_t *ptr);

uintptr_t* dictionary_get_hashes(dictionary_t* dict);
void dictionary_set_hashes(dictionary_t* dict, uintptr_t *ptr);

#endif /* DICTIONARY_H_ */
