#define _POSIX_C_SOURCE 200809L
#include "page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BufferPool *g_pool = NULL; 
void page_set_bufferpool(BufferPool *pool) {
    g_pool = pool;
    printf("[Page] Buffer pool set successfully\n");
}

//Initialize page
void page_init(BufferPool *pool) {
    if (!pool) {
        printf("[Page] Error: NULL buffer pool passed to page_init\n");
        return;
    }
    g_pool = pool;
    printf("[Page] Initialized with buffer pool reference\n");
}

Page *page_load(uint32_t page_id) {
    if (!g_pool) {
        printf("[Page] Error: buffer pool not initialized\n");
        return NULL;
    }

    PageFrame *frame = bufferpool_get_page(g_pool, page_id);
    if (!frame) {
        printf("[Page] Error: failed to get page %u from buffer pool\n", page_id);
        return NULL;
    }

    return (Page *)frame;
}

//Unpin a page
void page_unpin(Page *page) {
    if (!g_pool || !page) return;
    bufferpool_unpin_page(g_pool, page->page_id);
}

//Mark a page as dirty
void page_mark_dirty(Page *page) {
    if (!page) return;
    page->dirty = true;
}

//Force flush
void page_flush(Page *page) {
    if (!g_pool || !page) return;
    bufferpool_flush_page(g_pool, page->page_id);
}