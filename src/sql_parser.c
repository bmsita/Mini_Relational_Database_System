#include "sql_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char cols[8][32];
    int ncols;
    int where_id;
    int has_where;
} ColListCtx;

static void trim(char *str) {
    if (!str) return;
    char *p = str;
    while (isspace((unsigned char)*p)) p++;
    if (p != str) memmove(str, p, strlen(p)+1);
    if (*str == '\0') return;
    char *end = str + strlen(str) - 1;
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
    if (!r || !c) return;
    if (c->has_where && r->id != c->where_id) return;

    if (c->ncols == 0) {
        printf("%d | %-10s | %d\n", r->id, r->name, r->marks);
        return;
    }
    for (int i = 0; i < c->ncols; ++i) {
        if (strcasecmp(c->cols[i], "id") == 0)
            printf("%d", r->id);
        else if (strcasecmp(c->cols[i], "name") == 0)
            printf("%s", r->name);
        else if (strcasecmp(c->cols[i], "marks") == 0 || strcasecmp(c->cols[i], "score") == 0)
            printf("%d", r->marks);
        else
            printf("%s", "NULL");

        if (i + 1 < c->ncols) printf(" | ");
    }
    printf("\n");
}

void sql_execute(const char *query, BPTree *index) {
    char cmd[512];
    strncpy(cmd, query, sizeof(cmd)-1);
    cmd[sizeof(cmd)-1] = '\0';
    trim(cmd);
    if (strlen(cmd) == 0) return;

    char *semi = strchr(cmd, ';');
    if (semi) *semi = '\0';
    trim(cmd);


    if (strncasecmp(cmd, "SELECT", 6) == 0) {
        const char *where_ptr = strcasestr(cmd, "WHERE");
        int where_id = -1, has_where = 0;

        if (where_ptr && sscanf(where_ptr, "WHERE id = %d", &where_id) == 1)
            has_where = 1;

        const char *from = strcasestr(cmd, "FROM");
        if (!from) {
            const char *expr = cmd + 6;
            while (isspace((unsigned char)*expr)) expr++;
            char expr_copy[128];
            strncpy(expr_copy, expr, sizeof(expr_copy) - 1);
            expr_copy[sizeof(expr_copy)-1] = '\0';
            int result = evaluate_expression(expr_copy);
            printf("Result: %d\n", result);
            return;
        }

        char cols_buf[128];
        size_t cols_len = (size_t)(from - (cmd + 6));
        if (cols_len >= sizeof(cols_buf)) cols_len = sizeof(cols_buf) - 1;
        strncpy(cols_buf, cmd + 6, cols_len);
        cols_buf[cols_len] = '\0';
        trim(cols_buf);

        char tablename[64];
        sscanf(from, "FROM %63s", tablename);
        if (strcasecmp(tablename, "students") != 0) {
            printf("Only table 'students' supported in this demo.\n");
            return;
        }

        ColListCtx ctx = { .ncols = 0, .has_where = has_where, .where_id = where_id };

        if (strcmp(cols_buf, "*") != 0) {
            char *tok = strtok(cols_buf, ",");
            while (tok && ctx.ncols < 8) {
                trim(tok);
                strncpy(ctx.cols[ctx.ncols], tok, 31);
                ctx.cols[ctx.ncols][31] = '\0';
                ctx.ncols++;
                tok = strtok(NULL, ",");
            }
        }

        if (ctx.ncols == 0) {
            printf("\nID | Name       | Marks\n");
            printf("-------------------------\n");
        } else {
            for (int i = 0; i < ctx.ncols; ++i) {
                printf("%s", ctx.cols[i]);
                if (i + 1 < ctx.ncols) printf(" | ");
            }
            printf("\n");
        }

        bptree_for_each(index, print_record_cols, &ctx);
        return;
    }

    else if (strncasecmp(cmd, "INSERT", 6) == 0) {
        int id, marks;
        char name[128];
        if (sscanf(cmd, "INSERT INTO students VALUES (%d, '%127[^']', %d)", &id, name, &marks) == 3 ||
            sscanf(cmd, "INSERT INTO students VALUES (%d, \"%127[^\"]\", %d)", &id, name, &marks) == 3) {
            Record *rec = malloc(sizeof(Record));
            rec->id = id;
            strncpy(rec->name, name, sizeof(rec->name)-1);
            rec->name[sizeof(rec->name)-1] = '\0';
            rec->marks = marks;
            bptree_insert(index, rec->id, rec);
            printf("Record inserted: ID=%d, Name=%s, Marks=%d\n", rec->id, rec->name, rec->marks);
            return;
        } else {
            printf("Invalid INSERT syntax.\nExample: INSERT INTO students VALUES (1, 'Rojalin', 95);\n");
            return;
        }
    }

    else if (strncasecmp(cmd, "DELETE FROM students WHERE id", 29) == 0) {
        int id;
        if (sscanf(cmd, "DELETE FROM students WHERE id = %d", &id) == 1) {
            bptree_delete(index, id);
            printf("[B+Tree] Deleted key=%d\n", id);
            printf("Deleted record(s) with ID=%d\n", id);
        } else {
            printf("Invalid DELETE syntax.\nExample: DELETE FROM students WHERE id = 5;\n");
        }
        return;
    }

    else if (strncasecmp(cmd, "UPDATE students SET", 19) == 0) {
        int id;
        char name[50] = "";
        int marks = -1;

        char *set_ptr = strcasestr(cmd, "SET");
        char *where_ptr = strcasestr(cmd, "WHERE");

        if (!where_ptr || sscanf(where_ptr, "WHERE id = %d", &id) != 1) {
            printf("Invalid or missing WHERE clause.\nExample: UPDATE students SET marks = 90 WHERE id = 2;\n");
            return;
        }

        char set_part[128];
        strncpy(set_part, set_ptr + 3, where_ptr - (set_ptr + 3));
        set_part[where_ptr - (set_ptr + 3)] = '\0';
        trim(set_part);

        char *tok = strtok(set_part, ",");
        while (tok) {
            trim(tok);
            if (sscanf(tok, "name = '%49[^']'", name) == 1 ||
                sscanf(tok, "name = \"%49[^\"]\"", name) == 1) {
            } else if (sscanf(tok, "marks = %d", &marks) == 1) {
            }
            tok = strtok(NULL, ",");
        }

        Record *r = bptree_search(index, id);
        if (!r) {
            printf("No record found with ID=%d\n", id);
            return;
        }

        if (strlen(name) > 0) strcpy(r->name, name);
        if (marks >= 0) r->marks = marks;

        printf("Updated record ID=%d -> Name=%s, Marks=%d\n", r->id, r->name, r->marks);
        return;
    }

    else if (strncasecmp(cmd, "HELP", 4) == 0) {
        printf("\nAvailable Commands:\n");
        printf("  SELECT <expr>;                      -> Evaluate arithmetic\n");
        printf("  SELECT * FROM students;             -> Display all student records\n");
        printf("  SELECT name FROM students;          -> Display name column\n");
        printf("  SELECT * FROM students WHERE id=n;  -> Filter by ID\n");
        printf("  INSERT INTO students VALUES (id, 'name', marks)\n");
        printf("  DELETE FROM students WHERE id=n;\n");
        printf("  UPDATE students SET marks=95 WHERE id=n;\n");
        printf("  HELP;                               -> Show help message\n");
        printf("  EXIT;                               -> Quit program\n\n");
        return;
    }

    else if (strncasecmp(cmd, "EXIT", 4) == 0) {
        printf("Exiting MiniDB...\n");
        exit(0);
    }

    printf("Unknown command, Type HELP for help.\n");
}