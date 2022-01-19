#ifndef XTETRIS_UTIL_H
#define XTETRIS_UTIL_H

#include <stddef.h>
#include <stdlib.h>

void * malloc_or_die(size_t sz);
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
