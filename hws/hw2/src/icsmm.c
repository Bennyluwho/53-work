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
    ics_header *h = NULL;
    ics_footer *f = NULL;

    if (!valid_allocated_block(ptr, &h, &f)) {
        errno = EINVAL;
        return -1;
    }

    size_t bsz = GET_SIZE(h->block_size);
    h->block_size = PACK(bsz, 0, 0);
    f->block_size = PACK(bsz, 0, 0);

    ics_free_header *free_blk = coalesce(h);
    free_blk->next = NULL;
    free_blk->prev = NULL;

    insert_free_block_ordered(free_blk);
    return 0; 
}