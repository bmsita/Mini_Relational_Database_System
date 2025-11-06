//src/wal.c
#include "wal.h"
#include <string.h>
#include <stdlib.h>
WAL g_wal;

void wal_init(WAL *wal, const char *filename) {
    wal->log_file = fopen(filename, "ab+");
    if (!wal->log_file) {
        perror("WAL init failed");
        exit(1);
    }
    printf("[WAL] Initialized: %s\n", filename);
}

void wal_log_write(WAL *wal, int page_id, int offset, const void *data, size_t len) {
    if (!wal || !wal->log_file) return;
    fwrite(&page_id, sizeof(int), 1, wal->log_file);
    fwrite(&offset, sizeof(int), 1, wal->log_file);
    fwrite(&len, sizeof(size_t), 1, wal->log_file);
    fwrite(data, len, 1, wal->log_file);
    fflush(wal->log_file);
    printf("[WAL] Logged write page=%d offset=%d len=%zu\n", page_id, offset, len);
}

void wal_flush(WAL *wal) {
    if (wal && wal->log_file) fflush(wal->log_file);
    printf("[WAL] Flushed\n");
}

void wal_recover(WAL *wal) {
    if (!wal || !wal->log_file) return;
    rewind(wal->log_file);
    printf("[WAL] Recovery started...\n");

    int page_id, offset;
    size_t len;
    uint8_t buffer[512];

    while (fread(&page_id, sizeof(int), 1, wal->log_file)) {
        fread(&offset, sizeof(int), 1, wal->log_file);
        fread(&len, sizeof(size_t), 1, wal->log_file);
        fread(buffer, len, 1, wal->log_file);
        printf("→ Replay: page %d offset %d len %zu\n", page_id, offset, len);
    }

    printf("[WAL] Recovery done\n");
}

void wal_close(WAL *wal) {
    if (wal && wal->log_file) {
        fclose(wal->log_file);
        wal->log_file = NULL;
        printf("[WAL] Closed\n");
    }
}
