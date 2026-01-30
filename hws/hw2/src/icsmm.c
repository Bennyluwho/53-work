/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 * If you want to make helper functions, put them in helpers.c
 */
#include "icsmm.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

ics_free_header *freelist_head = NULL;

void *ics_malloc(size_t size) { 
    if (size == 0) {
        errno = EINVAL;
        return NULL;
    }

    if (!heap_init) {
        if (init_heap_first_page() < 0) return NULL;
    }
    
    size_t needed = ALIGN(size) + sizeof(ics_header) + sizeof(ics_footer);
    size_t min = sizeof(ics_free_header) + sizeof(ics_footer);

    if (needed < min) {
        needed = min;
    }

    ics_free_header *blk = find_fit(needed);

    if (blk == NULL) {
        if (grow_heap_until_fit(needed) < 0) return NULL;
        blk = find_fit(needed);
    }
    
    if (blk == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    return place_block(blk, needed, size);
}

void *ics_realloc(void *ptr, size_t size) { 
    return NULL; 
}

int ics_free(void *ptr) { 
    return -1; 
}
