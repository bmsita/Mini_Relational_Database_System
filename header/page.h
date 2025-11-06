//header/page.h
#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include "buffer_pool.h"

typedef PageFrame Page;

void page_init(BufferPool *pool);

#endif