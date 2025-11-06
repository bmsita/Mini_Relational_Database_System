// header/record.h
#ifndef RECORD_H
#define RECORD_H

#include <stdint.h>

typedef struct {
    int id;
    char name[50];
    int marks;
} Record;


void record_init(void *arg); 
void record_write(int record_id, const Record *rec);
void record_read(int record_id, Record *out);

#endif 
