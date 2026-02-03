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
     if (ptr == NULL) {
        return ics_malloc(size);
    }

    ics_header *h = NULL;
    ics_footer *f = NULL;

    if (!valid_allocated_block(ptr, &h, &f)) {
        errno = EINVAL;
        return NULL;
    }

    if (size == 0) {
        ics_free(ptr);
        return NULL;
    }

    size_t H = sizeof(ics_header);
    size_t F = sizeof(ics_footer);

    size_t blk_size = GET_SIZE(h->block_size);
    size_t old_payload = blk_size - H - F - (size_t)h->padding_amount;

    //compute new needed block size (same as malloc)
    size_t needed = ALIGN(size) + H + F;

    //min block size to avoid illegal tiny allocated blocks
    size_t min_block = sizeof(ics_free_header) + F;
    if (needed < min_block) needed = min_block;

    //CASE 1: request is larger ALWAYS allocate new block
    if (size > old_payload) {
        void *newp = ics_malloc(size);
        if (newp == NULL) {
           //ics_malloc sets errno to ENOMEM as needed
            return NULL;
        }

        //copy old_payload bytes (the amount actually valid)
        char *src = (char *)ptr;
        char *dst = (char *)newp;
        size_t i = 0;
        while (i < old_payload) {
            *(dst + i) = *(src + i);
            i++;
        }

        ics_free(ptr);
        return newp;
    }

    //CASE 2: request is smaller or equal then keep same block, maybe split
    if (needed <= blk_size) {
        size_t remainder = blk_size - needed;
        size_t min_free = sizeof(ics_free_header) + F;

        //if remainder would form a valid free block split
        if (remainder >= min_free) {
            size_t pad = ALIGN(size) - size;
            int pbit = (pad > 0);

            //shrink current allocated block header/footer
            h->padding_amount = (uint64_t)pad;
            h->block_size = PACK(needed, 1, pbit);

            ics_footer *new_af = (ics_footer *)((char *)h + needed - F);
            new_af->fid = FOOTER_MAGIC;
            new_af->block_size = PACK(needed, 1, pbit);

            //create the remainder free block immediately after
            ics_free_header *rem = (ics_free_header *)((char *)h + needed);
            rem->header.hid = HEADER_MAGIC;
            rem->header.padding_amount = 0;
            rem->header.block_size = PACK(remainder, 0, 0);

            rem->next = NULL;
            rem->prev = NULL;

            ics_footer *rem_f = (ics_footer *)((char *)rem + remainder - F);
            rem_f->fid = FOOTER_MAGIC;
            rem_f->block_size = PACK(remainder, 0, 0);

            insert_free_block_ordered(rem);
        } else {
            //no split just update padding bits/amount to reflect new requested size
            size_t payload_cap = blk_size - H - F;
            size_t pad = payload_cap - size;
            int pbit = (pad > 0);

            h->padding_amount = (uint64_t)pad;
            h->block_size = PACK(blk_size, 1, pbit);

            f->block_size = PACK(blk_size, 1, pbit);
        }

        return ptr;
    }


    errno = EINVAL;
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