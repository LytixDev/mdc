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

#include <stdlib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* types */
typedef void * (*malloc_perchance) (size_t);
typedef void * (*realloc_perchance) (void *, size_t);
typedef void * (*calloc_perchance) (size_t, size_t);
typedef void (*free_perchance) (void *);

/* functions */
void *mdc_malloc(size_t size, const char *file, const int line);
void *mdc_calloc(size_t nitems, size_t size, const char *file, const int line);
void *mdc_realloc(void *ptr, size_t size, const char *file, const int line);
void mdc_free(void *ptr, const char *file, const int line);

void mdc_init();
void mdc_destroy();
void mdc_debug();

/* macros */
#define malloc(m) mdc_malloc(m, __FILE__, __LINE__)
#define calloc(m, n) mdc_calloc(m, n, __FILE__, __LINE__)
#define realloc(m, n) mdc_realloc(m, n, __FILE__, __LINE__)
#define free(m) mdc_free(m, __FILE__, __LINE__)

#endif /* MDC_H */


#ifdef MDC_IMPLEMENTATION
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <dlfcn.h>

#ifndef NICC_HT_IMPLEMENTATION
#define NICC_HT_IMPLEMENTATION
/* we do not want to track allocations in the hashtable implementation */
#undef malloc
#undef free
#undef realloc
#undef calloc
#include "nicc/nicc.h"
#define malloc(m) mdc_malloc(m, __FILE__, __LINE__)
#define calloc(m, n) mdc_calloc(m, n, __FILE__, __LINE__)
#define realloc(m, n) mdc_realloc(m, n, __FILE__, __LINE__)
#define free(m) mdc_free(m, __FILE__, __LINE__)


/* types */
struct alloc_record {
    size_t size;
    char *file;
    int line;
};

/* globals (booooo anti-pattern !!!!) */
/*
 * hashtable storing all allocations that have not been freed
 * key is the address
 * value is of type alloc_record
 * */
struct ht_t allocs;
malloc_perchance sysmalloc;
realloc_perchance sysrealloc;
calloc_perchance syscalloc;
free_perchance sysfree;

/* functions */
static void save_allocation(void *address, size_t size, const char *file, const int line)
{
    struct alloc_record *record = sysmalloc(sizeof(struct alloc_record));
    record->size = size;
    record->file = (char *)file;
    record->line = line;

    ht_set(&allocs, address, sizeof(void *), record, sizeof(struct alloc_record), NULL);
}

static void remove_allocation(void *address)
{
    ht_rm(&allocs, address, sizeof(void *));
}

void *mdc_malloc(size_t size, const char *file, const int line)
{
    void *p = sysmalloc(size);
    save_allocation(&p, size, file, line);
    return p;
}

void *mdc_realloc(void *ptr, size_t size, const char *file, const int line)
{
    void *p = sysrealloc(ptr, size);
    remove_allocation(&p);
    save_allocation(&p, size, file, line);
    return p;
}

void *mdc_calloc(size_t nitems, size_t size, const char *file, const int line)
{
    void *p = syscalloc(nitems, size);
    save_allocation(&p, size, file, line);
    return p;
}

void mdc_free(void *ptr, const char *file, const int line)
{
    remove_allocation(&ptr);
    sysfree(ptr);
}

void mdc_init()
{
    allocs = *ht_malloc(8);
    sysmalloc = dlsym(RTLD_NEXT, "malloc");
    sysrealloc = dlsym(RTLD_NEXT, "realloc");
    syscalloc = dlsym(RTLD_NEXT, "calloc");
    sysfree = dlsym(RTLD_NEXT, "free");

    /* panic */
    if (sysmalloc == NULL || sysfree == NULL)
        fprintf(stderr, "Could not find linked symbol for malloc or free.");
}

void mdc_destroy()
{
    ht_free(&allocs);
}

void mdc_debug()
{
    struct ht_item_t **raw = ht_raw(&allocs);
    struct alloc_record *record;
    void *ptr;
    for (size_t i = 0; raw[i] != NULL; i++) {
        ptr = raw[i]->key;
        record = raw[i]->value;
        printf("LEAK: %p. Allocation in file %s on line %d of size %zub.\n", ptr, record->file,
               record->line, record->size);
    }

    free(raw);
}
#endif /* MDC_IMPLEMENTATION */
#endif /* NICC_HT_IMPLEMENTATION */
