#include "sql_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char cols[4][32];
    int ncols;
} ColListCtx;

static void trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

static int evaluate_expression(const char *expr) {
    int a, b;
    char op;
    if (sscanf(expr, "%d %c %d", &a, &op, &b) == 3) {
        if (op == '+') return a + b;
        if (op == '-') return a - b;
        if (op == '*') return a * b;
        if (op == '/' && b != 0) return a / b;
    }
    return atoi(expr);
}

static void print_record_cols(Record *r, void *ctx) {
    ColListCtx *c = (ColListCtx*)ctx;
    if (c->ncols == 0) {
        printf("%d | %-10s | %d\n", r->id, r->name, r->marks);
        return;
    }
    for (int i = 0; i < c->ncols; ++i) {
        if (strcasecmp(c->cols[i], "id") == 0) {
            printf("%d", r->id);
        } else if (strcasecmp(c->cols[i], "name") == 0) {
            printf("%s", r->name);
        } else if (strcasecmp(c->cols[i], "marks") == 0 || strcasecmp(c->cols[i], "score") == 0) {
            printf("%d", r->marks);
        } else {
            printf("%s", "NULL");
        }
        if (i + 1 < c->ncols) printf(" | ");
    }
    printf("\n");
}
 
void sql_execute(const char *query, BPTree *index) {
    char cmd[256];
    strcpy(cmd, query);
    trim(cmd);
    if (strlen(cmd) == 0) return;
    
    char *semi = strchr(cmd, ';');
    if (semi) *semi = '\0';
    trim(cmd);

    if (strncasecmp(cmd, "SELECT", 6) == 0) {
        if (strncasecmp(cmd, "SELECT * FROM students", 21) == 0) {
            printf("\nID | Name     | Marks\n");
            printf("-----------------------\n");
            ColListCtx ctx = { .ncols = 0 };
            bptree_for_each(index, print_record_cols, &ctx);
            return;
        }

    
        const char *after_sel = cmd + 6;
        while (isspace((unsigned char)*after_sel)) after_sel++;

    
        char *from = strcasestr(after_sel, "FROM");
        if (from) {
            
            size_t cols_len = (size_t)(from - after_sel);
            char cols_buf[128];
            if (cols_len >= sizeof(cols_buf)) cols_len = sizeof(cols_buf) - 1;
            strncpy(cols_buf, after_sel, cols_len);
            cols_buf[cols_len] = '\0';
            trim(cols_buf);

            // check table name
            char tablename[64];
            sscanf(from, "FROM %63s", tablename);
            if (strcasecmp(tablename, "students") != 0) {
                printf("Only table 'students' supported in this demo.\n");
                return;
            }
            ColListCtx ctx;
            ctx.ncols = 0;
            char *tok = strtok(cols_buf, ",");
            while (tok && ctx.ncols < 4) {
                trim(tok);
                strncpy(ctx.cols[ctx.ncols], tok, 31);
                ctx.cols[ctx.ncols][31] = '\0';
                ctx.ncols++;
                tok = strtok(NULL, ",");
            }

            // header print
            if (ctx.ncols == 0) {
                printf("\nID | Name     | Marks\n");
                printf("-----------------------\n");
            } else {
                for (int i = 0; i < ctx.ncols; ++i) {
                    printf("%s", ctx.cols[i]);
                    if (i + 1 < ctx.ncols) printf(" | ");
                }
                printf("\n");
                for (int i = 0; i < ctx.ncols * 3; ++i) printf("-");
                printf("\n");
            }
            bptree_for_each(index, print_record_cols, &ctx);
            return;
        }
        const char *expr = cmd + 6;
        while (isspace((unsigned char)*expr)) expr++;
        char expr_copy[128];
        strncpy(expr_copy, expr, sizeof(expr_copy) - 1);
        expr_copy[sizeof(expr_copy)-1] = '\0';
        int result = evaluate_expression(expr_copy);
        printf("Result: %d\n", result);
        return;
    }
    else if (strncasecmp(cmd, "INSERT", 6) == 0) {
        int id, marks;
        char name[50];
        if (sscanf(cmd, "INSERT INTO students VALUES (%d, '%49[^']', %d)", &id, name, &marks) == 3 ||
            sscanf(cmd, "INSERT INTO students VALUES (%d, \"%49[^\"]\", %d)", &id, name, &marks) == 3) {
            // allocate record on heap
            Record *rec = malloc(sizeof(Record));
            rec->id = id;
            strncpy(rec->name, name, sizeof(rec->name)-1);
            rec->name[sizeof(rec->name)-1] = '\0';
            rec->marks = marks;
            bptree_insert(index, rec->id, rec);
            printf("Record inserted: ID=%d, Name=%s, Marks=%d\n", rec->id, rec->name, rec->marks);
        } else {
            printf("Invalid INSERT syntax.\nExample: INSERT INTO students VALUES (1, 'Alice', 95)\n");
        }
        return;
    }
else if (strncasecmp(cmd, "HELP", 4) == 0) {
        printf("\nAvailable Commands:\n");
        printf("  SELECT <expr>;               -> Evaluate arithmetic\n");
        printf("  SELECT * FROM students;      -> Display all student records\n");
        printf("  SELECT name FROM students;   -> Display name column\n");
        printf("  INSERT INTO students VALUES (id, 'name', marks)\n");
        printf("  HELP;                        -> Show help message\n");
        printf("  EXIT;                        -> Quit program\n\n");
        return;
    }

    else if (strncasecmp(cmd, "EXIT", 4) == 0) {
        printf("Exiting MiniDB...\n");
        exit(0);
    }

    printf("Unknown command,Type HELP for help.\n");
}