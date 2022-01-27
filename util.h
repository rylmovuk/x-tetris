/**
 * @file util.h
 * @author Maksim Kovalkov
 */

#ifndef XTETRIS_UTIL_H
#define XTETRIS_UTIL_H

#include <stddef.h>
#include <stdlib.h>

/**
 * Allocate an object of the given size, logging to `stderr` and exiting on failure.
 * @returns guaranteed non-null pointer to the allocated memory.
 */
void * malloc_or_die(size_t sz);

/**
 * Allocate `obj_count` independent objects of size `obj_size`.  
 * The return value points to `obj_count` pointers to each individual object.  
 * Logs to `stderr` and exits if any allocation fails.
 * 
 * @param  obj_count Number of objects to allocate.
 * @param  obj_size  Size of a single object.
 * @returns an array of (non-null) pointers to each allocated object.
 */
void * alloc_many_or_die(size_t, size_t);

/* typedef struct dynstr {
    char *data;
    unsigned size;
    unsigned capacity;
} dynstr_t;

void dynstr_init(dynstr_t *);
void dynstr_init_from(dynstr_t *, char *);
void dynstr_deinit(dynstr_t *);

void dynstr_set(dynstr_t *, char *);
void dynstr_setn(dynstr_t *, char *, size_t sz);
char const * dynstr_get(dynstr_t *);

void dynstr_copy(dynstr_t *dst, dynstr_t *src); */
#endif /* ifndef XTETRIS_UTIL_H */
