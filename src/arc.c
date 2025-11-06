#include "arc.h"
#include <stdio.h>
#include <stdlib.h>


static int capacity;
static int hits = 0, misses = 0, evictions = 0;

// Simple ARC placeholders for now
void arc_init(int cap) {
    capacity = cap;
    hits = misses = evictions = 0;
}

int arc_access_page(uint32_t page_id) {
    static uint32_t cache[32];
    static int size = 0;

    // Check hit
    for (int i = 0; i < size; i++) {
        if (cache[i] == page_id) {
            hits++;
            return 1; 
        }
    }

    // MISS
    misses++;
    if (size < capacity) {
        cache[size++] = page_id;
    } else {
        // Evict oldest
        for (int j = 1; j < size; j++)
            cache[j - 1] = cache[j];
        cache[size - 1] = page_id;
        evictions++;
    }
    return 0; // MISS
}

int arc_evict_page(void) {
    // Simplified victim selection
    if (capacity <= 0) return -1;
    evictions++;
    return 0; 
    // Return dummy victim index
}

void arc_print_state(void) {
    printf("[ARC] State -> Hits=%d Misses=%d Evictions=%d\n", hits, misses, evictions);
}

void arc_get_stats(int *h, int *m, int *e) {
    *h = hits;
    *m = misses;
    *e = evictions;
}

void arc_cleanup(void) {
    
}