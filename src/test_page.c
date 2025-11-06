#include "page.h"
#include "buffer_pool.h"
#include <stdio.h>

int main() {
    BufferPool pool;
    bufferpool_init(&pool, 3);

    page_set_bufferpool(&pool);
    page_init(&pool);

    Page *p1 = page_load(1);
    page_mark_dirty(p1);
    page_unpin(p1);
    page_flush(p1);

    bufferpool_shutdown(&pool);
    printf("[Test] Page module test complete.\n");
    return 0;
}
