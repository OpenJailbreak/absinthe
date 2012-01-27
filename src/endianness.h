/**
 * GreenPois0n Absinthe - endianness.h
 * Copyright (C) 2010 Chronic-Dev Team
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

#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif

#ifndef __BYTE_ORDER
#ifdef __LITTLE_ENDIAN__
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#ifdef __BIG_ENDIAN__
#define __BYTE_ORDER __BIG_ENDIAN
#endif
#endif
#endif

#ifndef be16toh
#if __BYTE_ORDER == __BIG_ENDIAN
#define be16toh(x) (x)
#else
#define be16toh(x) ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#endif
#endif

#ifndef htobe16
#define htobe16 be16toh
#endif

#ifndef __bswap_32
#define __bswap_32(x) ((((x) & 0xFF000000) >> 24) \
                    | (((x) & 0x00FF0000) >> 8) \
                    | (((x) & 0x0000FF00) << 8) \
                    | (((x) & 0x000000FF) << 24))
#endif

#ifndef be32toh
#if __BYTE_ORDER == __BIG_ENDIAN
#define be32toh(x) (x)
#else
#define be32toh(x) __bswap_32(x)
#endif
#endif

#ifndef htobe32
#define htobe32 be32toh
#endif

#ifndef __bswap_64
#define __bswap_64(x) ((((x) & 0xFF00000000000000ull) >> 56) \
                    | (((x) & 0x00FF000000000000ull) >> 40) \
                    | (((x) & 0x0000FF0000000000ull) >> 24) \
                    | (((x) & 0x000000FF00000000ull) >> 8) \
                    | (((x) & 0x00000000FF000000ull) << 8) \
                    | (((x) & 0x0000000000FF0000ull) << 24) \
                    | (((x) & 0x000000000000FF00ull) << 40) \
                    | (((x) & 0x00000000000000FFull) << 56)) 
#endif

#ifndef htobe64
#if __BYTE_ORDER == __BIG_ENDIAN
#define htobe64(x) (x)
#else
#define htobe64(x) __bswap_64(x)
#endif
#endif

#ifndef be64toh
#define be64toh htobe64
#endif

#ifndef le32toh
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le32toh(x) (x)
#else
#define le32toh(x) __bswap_32(x)
#endif
#endif

#ifndef htole32
#define htole32 le32toh
#endif

#ifndef le64toh
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le64toh(x) (x)
#else
#define le64toh(x) __bswap_64(x)
#endif
#endif

#ifndef htole64
#define htole64 le64toh
#endif

typedef enum {
	kBigEndian,
	kLittleEndian
} endian_t;

#endif /* ENDIANNESS_H */
