#ifndef HELPERS_H
#define HELPERS_H

#include "icsmm.h"
#include <errno.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 4

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define A_BIT 0x1ULL
#define P_BIT 0x2ULL
#define SIZE_MASK (~0xFULL)

#define GET_SIZE(x) ((size_t)((x) & SIZE_MASK))
#define PACK(sz, a, p) ((uint64_t)((sz) | ((a) ? A_BIT : 0) | ((p) ? P_BIT : 0)))

extern int heap_init;
extern int pages_allocated;


/* Helper function declarations go here */
int init_heap_first_page(void);
void *place_block(ics_free_header *blk, size_t needed, size_t size);
int grow_heap_until_fit(size_t needed);
ics_free_header *find_fit(size_t needed);


#endif
