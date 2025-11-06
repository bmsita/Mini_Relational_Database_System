//header/page.h
#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include "buffer_pool.h"

typedef PageFrame Page;

void page_init(BufferPool *pool);
void page_set_bufferpool(BufferPool *pool);
Page *page_load(uint32_t page_id);
void page_unpin(Page *page);
void page_mark_dirty(Page *page);
void page_flush(Page *page);

#endif