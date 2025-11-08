#ifndef STORAGE_H
#define STORAGE_H

#include "bptree.h"

void save_records_to_file(BPTree *index, const char *filename);
void load_records_from_file(BPTree *index, const char *filename);

#endif