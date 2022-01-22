#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

void *
malloc_or_die(size_t size)
{
    void *p = malloc(size);
    if (!p) {
        perror("cannot malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *
alloc_many_or_die(size_t obj_count, size_t obj_size)
{
    int i;
    void **p = malloc(obj_count * sizeof(void *));
    if (p) {
        for (i = 0; i < obj_count; ++i) {
            p[i] = malloc(obj_size);
            if (!p[i]) break;
        }

        /* success */
        if (i == obj_count)
            return p;

        /* if one object's allocation failed, free everything */
        while (i--) 
            free(p[i]);
        free(p);
    }

    /* either the array allocation or one of the object allocations failed */
    perror("malloc failed");
    exit(EXIT_FAILURE);
}
