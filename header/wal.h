//header/wal.h
#ifndef WAL_H
#define WAL_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *log_file;
} WAL;

void wal_init(WAL *wal, const char *filename);
void wal_log_write(WAL *wal, int page_id, int offset, const void *data, size_t len);
void wal_flush(WAL *wal);
void wal_recover(WAL *wal);
void wal_close(WAL *wal);
extern WAL g_wal; 
#endif
