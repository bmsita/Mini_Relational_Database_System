#include "sql_parser.h"
#include "storage.h" 
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>  

int main() {
    printf("MiniDB Starting......\n");
    BufferPool pool;
    bufferpool_init(&pool, 3);
    wal_init(&g_wal, "wal.log");
    record_init(&pool);

    BPTree index;
    bptree_init(&index);
    load_records_from_file(&index, "students.db");

    printf("MiniDB SQL Engine Ready\n");
    printf("Type SQL queries like:\n");
    printf("  SELECT 7 + 5;\n");
    printf("  INSERT INTO students VALUES (1, 'Rojalin', 95);\n");
    printf("  SELECT * FROM students;\n");
    printf("Type EXIT; to quit.\n\n");

    char query[256];
    while (1) {
        printf("SQL> ");
        if (!fgets(query, sizeof(query), stdin)) break;
        if (strncasecmp(query, "EXIT", 4) == 0) break;
        sql_execute(query, &index);
    }
    save_records_to_file(&index, "students.db");

    bptree_free(index.root);
    bufferpool_cleanup(&pool);
    wal_close(&g_wal);
    printf("MiniDB Shutdown complete. Data saved successfully.\n");
    
    return 0;
}