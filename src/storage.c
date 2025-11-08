#include "bptree.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_record_to_file(Record *r, void *ctx) {
    FILE *fp = (FILE *)ctx;
    fprintf(fp, "%d,%s,%d\n", r->id, r->name, r->marks);
}

void save_records_to_file(BPTree *index, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file for saving");
        return;
    }
    bptree_for_each(index, write_record_to_file, fp);
    fclose(fp);
    printf("[Storage] Records saved to %s\n", filename);
}

void load_records_from_file(BPTree *index, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("[Storage] No existing database found, starting new.\n");
        return;
    }

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        Record *rec = malloc(sizeof(Record));
        if (sscanf(line, "%d,%49[^,],%d", &rec->id, rec->name, &rec->marks) == 3) {
            bptree_insert(index, rec->id, rec);
        } else {
            free(rec);
        }
    }
    fclose(fp);
    printf("[Storage] Records loaded from %s\n", filename);
}