#ifndef ARC_H
#define ARC_H

#include <stdint.h>

void arc_init(int capacity);

int arc_access_page(uint32_t page_id);

int arc_evict_page(void);

void arc_print_state(void);

void arc_get_stats(int *hits, int *misses, int *evictions);

void arc_cleanup(void);

#endif