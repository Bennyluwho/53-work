#include "helpers.h"

int heap_init = 0;
int pages_allocated = 0;

/* Helper function definitions go here */
int init_heap_first_page(void) {
    void *page = ics_inc_brk();
    if (page == ((void *) -1)) return -1;
    pages_allocated++;

    //since a char is 1 byte this will act as my pointer to the start of the page
    char *base = (char *) page;

    size_t H = sizeof(ics_header);
    size_t F = sizeof(ics_footer);

    size_t pro_size = H + F + 8;

    //initalize prologue: header + footer = 16 bytes
    ics_header *pro_h = (ics_header*) base;
    pro_h->block_size = pro_size;
    pro_h->hid = HEADER_MAGIC;
    pro_h->padding_amount = 0;

    ics_footer *pro_f = (ics_footer *) (base + H);
    pro_f->block_size = pro_size;
    pro_f->fid = FOOTER_MAGIC;

    //initalize epilogue header at the end of page (8 bytes)
    ics_header *epi_h = (ics_header *) (base + PAGE_SIZE - H);
    epi_h->block_size = 0;
    epi_h->hid = HEADER_MAGIC;
    epi_h->padding_amount = 0;

    //initial free block ptr RIGHT AFTER the prologue block (16 bytes after the starting address)
    ics_free_header *free_blk = (ics_free_header *) (base + pro_size);

    //free block size is everything between the free_blk and epilouge
    size_t free_size = (size_t)((char *)epi_h - (char *)free_blk);

    free_blk->header.block_size = free_size;
    free_blk->header.hid = HEADER_MAGIC;
    free_blk->header.padding_amount = 0;

    free_blk->next = NULL;
    free_blk->prev = NULL;

    //initalize the free block's footer
    ics_footer *free_f = (ics_footer *) ((char *)free_blk + free_size - F);
    free_f->block_size = free_size;
    free_f-> fid = FOOTER_MAGIC;

    freelist_head = free_blk;

    heap_init = 1;
    return 0;
}

static void remove_free_block(ics_free_header *blk) {
    if(blk->prev != NULL) {
        blk->prev->next = blk->next;
    } else {
        freelist_head = blk->next;
    }

    if (blk->next != NULL) {
        blk->next->prev = blk->prev;
    }

    blk->next = NULL;
    blk->prev = NULL;
}

static void insert_free_block(ics_free_header *blk) {
    blk->prev = NULL;
    blk->next = freelist_head;

    if (freelist_head != NULL) {
        freelist_head->prev = blk;
    }

    freelist_head = blk;
}

ics_free_header *find_fit(size_t needed) {
    ics_free_header *curr = freelist_head;

    while (curr != NULL) {
        if (curr->header.block_size >= needed) return curr;
        curr = curr->next;
    }
    return NULL;
}

void *place_block(ics_free_header *blk, size_t needed, size_t size) {
    size_t H = sizeof(ics_header);
    size_t F = sizeof(ics_footer);

    //minimum size for a free block
    size_t min_free = sizeof(ics_free_header) + F;

    size_t blk_size = blk->header.block_size;

    //detach this block from the free list
    remove_free_block(blk);

    //choose to either split or no split
    if (blk_size >= needed + min_free) {
    //SPLITS
        
        //allocated block uses exactly what is 'needed' ;)
        blk->header.block_size = needed;
        blk->header.hid = HEADER_MAGIC;

        //padding = aligned_payload - requested_payload
        blk->header.padding_amount = (uint64_t)(ALIGN(size) - size);

        ics_footer *alloc_f = (ics_footer *)((char*)blk + needed - F);
        alloc_f->block_size = needed;
        alloc_f->fid = FOOTER_MAGIC;

        //remainder free block located right after allocated block
        ics_free_header *rem = (ics_free_header *)((char *)blk + needed);
        size_t rem_size = blk_size - needed;

        rem->header.block_size = rem_size;
        rem->header.hid = HEADER_MAGIC;
        rem->header.padding_amount = 0;

        ics_footer *rem_f = (ics_footer*)((char *)rem + rem_size - F);
        rem_f->block_size = rem_size;
        rem_f->fid = FOOTER_MAGIC;

        rem->next = NULL;
        rem->prev = NULL;

        insert_free_block(rem);
    } else {
    //no split, avoids splinters
        //allocate the WHOLE block
        blk->header.block_size = blk_size;
        blk->header.hid = HEADER_MAGIC;

        //payload cap in this full block
        size_t payload_cap = blk_size - (H + F);

        //padding = cap - requested (already checked to be <= capacity)
        blk->header.padding_amount = (uint64_t)(payload_cap - size);

        ics_footer *alloc_f = (ics_footer *)((char *)blk + blk_size - F);
        alloc_f->block_size = blk_size;
        alloc_f->fid = FOOTER_MAGIC;
    }

    return (void *)((char *)blk + H);
}

static void write_epilogue(void) {
    char *brk = (char *)ics_get_brk();
    ics_header *epi = (ics_header *)(brk - sizeof(ics_header));

    epi->block_size = 0;
    epi->hid = HEADER_MAGIC;
    epi->padding_amount = 0;
}

static int is_in_freelist(ics_free_header *blk) {
    ics_free_header *curr = freelist_head;
    while (curr != NULL) {
        if (curr == blk) return 1;
        curr = curr->next;
    }
    return 0;
}

int grow_heap_one_page(void) {
    if (pages_allocated >= MAX_PAGES) {
        errno = ENOMEM;
        return -1;
    }

    void *new_page = ics_inc_brk();
    if (new_page ==((void *)-1)) return -1;

    pages_allocated++;

    //after increment, the old epilogue header is now at the start of the new space
    //Now we want to extend the ;ast real block if free
    char *old_brk = (char *)new_page;
    ics_footer *last_f = (ics_footer *)(old_brk - sizeof(ics_header) - sizeof(ics_footer));
    size_t last_size = last_f->block_size;

    //block start is at the end of the block so block start = (footer_addr - block_size + footer_size)
    ics_free_header *last_blk = (ics_free_header *)((char*)last_f - last_size + sizeof(ics_footer));

    //if the last block is free then it should be in the free list*****
    if (is_in_freelist(last_blk)) {
        //extend the last free block by one page
        size_t new_size = last_blk->header.block_size + PAGE_SIZE;

        last_blk->header.block_size = new_size;
        last_blk->header.hid = HEADER_MAGIC;
        last_blk->header.padding_amount = 0;

        ics_footer *new_footer = (ics_footer *)((char *)last_blk + new_size - sizeof(ics_footer));
        new_footer->block_size = new_size;
        new_footer->fid = FOOTER_MAGIC;

        write_epilogue();
        return 0;
    }

    //OTHERWISE create a new free block out of the new page
    //header should start at teh OLD epilogue header location
    ics_free_header *fresh = (ics_free_header *)((char *)new_page - sizeof(ics_header));

    fresh->header.block_size = PAGE_SIZE + sizeof(ics_header);
    fresh->header.hid = HEADER_MAGIC;
    fresh->header.padding_amount = 0;

    fresh->next = NULL;
    fresh->prev = NULL;
    
    ics_footer *fresh_f = (ics_footer *)((char *)fresh + fresh->header.block_size - sizeof(ics_footer));
    fresh_f->block_size = fresh->header.block_size;
    fresh_f->fid = FOOTER_MAGIC;

    //insert into freelist
    insert_free_block(fresh);

    write_epilogue();
    return 0;
}

int grow_heap_until_fit(size_t needed) {
    while (find_fit(needed) == NULL) {
        if (grow_heap_one_page() < 0) return -1;
    }
    return 0;
}