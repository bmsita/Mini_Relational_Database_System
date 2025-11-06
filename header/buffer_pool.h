//header/buffer_pool.h
#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

typedef struct {
    uint32_t page_id;
    bool dirty;
    int pin_count;
    char data[PAGE_SIZE];
} PageFrame;

typedef struct {
    int capacity;
    int num_pages;
    PageFrame *frames; 
} BufferPool;

/* API */
void bufferpool_init(BufferPool *pool, int capacity);

#endif 