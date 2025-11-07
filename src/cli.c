#include "sql_parser.h"

WAL g_wal;

int main() {
    BufferPool pool;
    bufferpool_init(&pool, 3);
    wal_init(&g_wal, "wal.log");
    record_init(&pool);

    BPTree index;
    bptree_init(&index);

    printf("\nMiniDB SQL Engine Ready!\n");
    printf("Type SQL queries like:\n");
    printf("  SELECT 7 + 5;\n");
    printf("  INSERT INTO students VALUES (1, 'Rojalin', 95);\n");
    printf("  SELECT * FROM students;\n");
    printf("Type EXIT; to quit.\n\n");

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