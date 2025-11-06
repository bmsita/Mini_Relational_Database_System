#include "sql_parser.h"
#include <string.h>   

int main() {
    BufferPool pool;
    bufferpool_init(&pool, 3);
    wal_init(&g_wal, "wal.log");
    record_init(&pool);

    BPTree index;
    bptree_init(&index);

    printf("MiniDB SQL Engine Ready! Type SQL queries (e.g., SELECT 7+5;)\n");

    char query[256];
    while (1) {
        printf("SQL> ");
        if (!fgets(query, sizeof(query), stdin)) break;
        if (strncmp(query, "EXIT", 4) == 0) break;  
        sql_execute(query, &index);
    }

    bufferpool_cleanup(&pool);
    wal_close(&g_wal);
    return 0;
}