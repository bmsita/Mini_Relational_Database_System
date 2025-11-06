// src/record.c
#define _POSIX_C_SOURCE 200809L
#include "record.h"
#include "page.h"
#include <stdio.h>
#include <string.h>
#include "wal.h"

extern WAL g_wal;

#define RECORD_SIZE sizeof(Record)
#define RECORDS_PER_PAGE (PAGE_SIZE / RECORD_SIZE)

static BufferPool *g_pool_for_record = NULL;
void record_init(void *arg) {
    BufferPool *pool = (BufferPool *)arg;
    page_set_bufferpool(pool);
    g_pool_for_record = pool;
}


//Write fixed-size Record by record_id 
void record_write(int record_id, const Record *rec) {
    int page_id = record_id / RECORDS_PER_PAGE;
    int slot_index = record_id % RECORDS_PER_PAGE;
    int offset = (record_id % RECORDS_PER_PAGE) * RECORD_SIZE;
    

    //Log before modifying
    wal_log_write(&g_wal, page_id, offset, rec, sizeof(Record));

    Page *page = page_load(page_id);
    if (!page) {
        printf("[record_write] failed to load page %d\n", page_id);
        return;
    }
    memcpy(page->data + offset, rec, RECORD_SIZE);
    page_mark_dirty(page);
    page_unpin(page);
    printf("[record_write] wrote record %d to page %d offset %d\n", record_id, page_id, offset);
}

//Read fixed-size Record
void record_read(int record_id, Record *out) {
    int page_id = record_id / RECORDS_PER_PAGE;
    int slot_index = record_id % RECORDS_PER_PAGE;
    int offset = slot_index * RECORD_SIZE;

    Page *page = page_load(page_id);
    if (!page) {
        printf("[record_read] failed to load page %d\n", page_id);
        return;
    }
    memcpy(out, page->data + offset, RECORD_SIZE);
    page_unpin(page);
    printf("[record_read] read record %d from page %d offset %d\n", record_id, page_id, offset);
}