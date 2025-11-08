//sql_parser.h
#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include "record.h"
#include "bptree.h"
#include "buffer_pool.h"
#include "wal.h"

void sql_execute(const char *query, BPTree *index);

#endif
