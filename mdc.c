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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <dlfcn.h>

#define NICC_HT_IMPLEMENTATION
#include "nicc/nicc.h"
#define MDC_IMPLEMENTATION
#include "mdc.h"


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
free_perchance sysfree;


/* functions */
void *mdc_calloc(size_t nitems, size_t size, const char *file, const int line);
void *mdc_realloc(void *ptr, size_t size, const char *file, const int line);


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

void mdc_free(void *ptr, const char *file, const int line)
{
    remove_allocation(&ptr);
    sysfree(ptr);
}


void mdc_init()
{
    allocs = *ht_malloc(8);
    sysmalloc = dlsym(RTLD_NEXT, "malloc");
    sysfree = dlsym(RTLD_NEXT, "free");

    /* panic */
    if (sysmalloc == NULL || sysfree == NULL) {
        fprintf(stderr, "Could not find linked symbol for malloc or free");
        #undef malloc
        #undef free
        #undef realloc
        #undef calloc
    }

}

void mdc_destroy()
{
    ht_free(&allocs, true);
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
