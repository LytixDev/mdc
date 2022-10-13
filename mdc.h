/*
 *  Copyright (C) 2022 Nicolai Brand 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MDC_H
#define MDC_H


#ifdef MDC_IMPLEMENTATION
#       ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#       endif
#include <stdlib.h>

typedef void * (*malloc_perchance) (size_t);
typedef void (*free_perchance) (void *);


void *mdc_malloc(size_t size, const char *file, const int line);
void *mdc_calloc(size_t nitems, size_t size, const char *file, const int line);
void *mdc_realloc(void *ptr, size_t size, const char *file, const int line);
void mdc_free(void *ptr, const char *file, const int line);

void mdc_init();
void mdc_destroy();
void mdc_debug();

#define malloc(m) mdc_malloc(m, __FILE__, __LINE__)
#define calloc(m, n) mdc_calloc(m, n, __FILE__, __LINE__)
#define realloc(m, n) mdc_realloc(m, n, __FILE__, __LINE__)
#define free(m) mdc_free(m, __FILE__, __LINE__)

#endif /* MDC_IMPLEMENTATION */
#endif /* MDC_H */
