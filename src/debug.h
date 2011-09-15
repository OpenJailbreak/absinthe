/*
 * debug.h
 *
 *  Created on: Sep 12, 2011
 *      Author: posixninja
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdarg.h>

#ifdef _DEBUG
#define info(...) fprintf(stdout, __VA_ARGS__)
#define error(...) fprintf(stderr, __VA_ARGS__)
#define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define info(...) printf(__VA_ARGS__)
#define error(...) fprintf(stderr, __VA_ARGS__)
#define debug(...)
#endif

#endif /* DEBUG_H_ */
