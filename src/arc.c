//src/arc.c
#include "arc.h"
#include <stdio.h>
#include <stdlib.h>


static int capacity;
static int total_hits = 0;
static int total_misses = 0;
static int total_evictions = 0;

// Simple ARC placeholders for now
void arc_init(int cap) {
    capacity = cap;
    total_hits =total_misses = total_evictions = 0;
}

int arc_access_page(uint32_t page_id) {
    static uint32_t cache[32];
    static int size = 0;

    // Check hit
    for (int i = 0; i < size; i++) {
        if (cache[i] == page_id) {
            total_hits++;
            return 1; 
        }
    }

    // MISS
    total_misses++;
    if (size < capacity) {
        cache[size++] = page_id;
    } else {
        // Evict oldest
        for (int j = 1; j < size; j++)
            cache[j - 1] = cache[j];
        cache[size - 1] = page_id;
        total_evictions++;
    }
    return 0; // MISS
}

int arc_evict_page(void) {
    // Simplified victim selection
    if (capacity <= 0) return -1;
    total_evictions++;
    return 0; 
    // Return dummy victim index
}

void arc_print_state(void) {
    printf("[ARC] State -> Hits=%d Misses=%d Evictions=%d\n", total_hits, total_misses, total_evictions);
}

void arc_get_stats(int *hits, int *misses, int *evictions) {
    if (hits) *hits = total_hits;
    if (misses) *misses = total_misses;
    if (evictions) *evictions = total_evictions;

    printf("[ARC] Stats -> Hits: %d | Misses: %d | Evictions: %d\n", 
           total_hits, total_misses, total_evictions);
}

void arc_cleanup(void) {
    
}