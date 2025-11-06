// src/buffer_pool.c
#define _POSIX_C_SOURCE 200809L
#include "buffer_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Initialize pool
void bufferpool_init(BufferPool *pool, int capacity) {
    if (!pool) return;
    pool->capacity = capacity;
    pool->num_pages = 0;
    pool->frames = (PageFrame*)calloc(capacity, sizeof(PageFrame));
    if (!pool->frames) {
        perror("calloc frames");
        exit(1);
    }
    for (int i = 0; i < capacity; ++i) {
        pool->frames[i].page_id = UINT32_MAX; // unused
        pool->frames[i].dirty = false;
        pool->frames[i].pin_count = 0;
        memset(pool->frames[i].data, 0, PAGE_SIZE);
    }
    printf("[BufferPool] Initialized with %d frames.\n", capacity);
}

PageFrame *bufferpool_get_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return NULL;
    int idx = find_index(pool, page_id);
    if (idx != -1) {
        pool->frames[idx].pin_count++;
        printf("[HIT] Page %u found in frame %d (pin_count=%d)\n",
               page_id, idx, pool->frames[idx].pin_count);
        return &pool->frames[idx];
    }

    int slot = find_free_or_evict(pool);
    if (slot == -1) {
        printf("[ERROR] No evictable frame available. Cannot load page %u.\n", page_id);
        return NULL;
    }


    if (pool->frames[slot].page_id != UINT32_MAX) {
        if (pool->frames[slot].dirty) {
            simulated_disk_write(pool->frames[slot].page_id, pool->frames[slot].data);
        }
    }
}

//unpin pages
void bufferpool_unpin_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return;
    int idx = find_index(pool, page_id);
    if (idx == -1) {
        printf("[WARN] unpin: page %u not found\n", page_id);
        return;
    }
    if (pool->frames[idx].pin_count > 0) pool->frames[idx].pin_count--;
    printf("[UNPIN] Page %u (pin_count=%d)\n", page_id, pool->frames[idx].pin_count);
}

//Cleanup
void bufferpool_cleanup(BufferPool *pool) {
    if (!pool) return;
    for (int i = 0; i < pool->capacity; ++i) {
        if (pool->frames[i].page_id != UINT32_MAX && pool->frames[i].dirty) {
            simulated_disk_write(pool->frames[i].page_id, pool->frames[i].data);
        }
    }
    free(pool->frames);
    pool->frames = NULL;
    pool->num_pages = 0;
    printf("[BufferPool] Cleaned up.\n");
}