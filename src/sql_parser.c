//src/sql_parser.c
#include "sql_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char cols[8][32];
    int ncols;

    int has_where;
    char where_col[32];
    char where_op[4];
    int where_is_str;
    char where_str[128];
    int where_val;

    int has_order;
    char order_col[32];
    char order_dir[8];
} ColListCtx;

static void trim(char *str) {
    if (!str) return;
    char *p = str;
    while (isspace((unsigned char)*p)) p++;
    if (p != str) memmove(str, p, strlen(p) + 1);
    if (*str == '\0') return;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

static int evaluate_expression(const char *expr) {
    if (!expr) return 0;
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

typedef struct {
    Record *records[1024];
    int count;
} ResultSet;

static void collect_record(Record *r, void *ctx) {
    ResultSet *res = (ResultSet*)ctx;
    if (res->count < 1024) res->records[res->count++] = r;
}

static int compare_record(Record *a, Record *b, const char *col, const char *dir) {
    int cmp = 0;
    if (strcasecmp(col, "id") == 0) cmp = a->id - b->id;
    else if (strcasecmp(col, "marks") == 0) cmp = a->marks - b->marks;
    else if (strcasecmp(col, "name") == 0) cmp = strcmp(a->name, b->name);

    if (strcasecmp(dir, "DESC") == 0) cmp = -cmp;
    return cmp;
}

void sql_execute(const char *query, BPTree *index) {
    char cmd[1024];
    if (!query || !index) return;
    strncpy(cmd, query, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    trim(cmd);
    if (strlen(cmd) == 0) return;

    char *semi = strchr(cmd, ';');
    if (semi) *semi = '\0';
    trim(cmd);

    if (strncasecmp(cmd, "SELECT", 6) == 0) {
        const char *from = strcasestr(cmd, "FROM");
        const char *where_ptr = strcasestr(cmd, "WHERE");
        const char *order_ptr = strcasestr(cmd, "ORDER BY");

        if (!from) {
            const char *expr = cmd + 6;
            while (isspace((unsigned char)*expr)) expr++;
            char expr_copy[256];
            strncpy(expr_copy, expr, sizeof(expr_copy)-1);
            expr_copy[sizeof(expr_copy)-1] = '\0';
            trim(expr_copy);
            int result = evaluate_expression(expr_copy);
            printf("Result: %d\n", result);
            return;
        }

        // columns
        char cols_buf[256];
        size_t cols_len = (size_t)(from - (cmd + 6));
        if (cols_len >= sizeof(cols_buf)) cols_len = sizeof(cols_buf) - 1;
        strncpy(cols_buf, cmd + 6, cols_len);
        cols_buf[cols_len] = '\0';
        trim(cols_buf);

        // table name
        char tablename[64] = {0};
        sscanf(from, "FROM %63s", tablename);
        if (strcasecmp(tablename, "students") != 0) {
            printf("Only table 'students' supported in this demo.\n");
            return;
        }

        ColListCtx ctx = {0};
        ctx.ncols = 0;
        ctx.has_where = 0;
        ctx.has_order = 0;
        ctx.where_is_str = 0;

        if (where_ptr) {
            const char *where_start = where_ptr + 5;
            const char *where_end = order_ptr ? order_ptr : cmd + strlen(cmd);
            size_t wlen = (size_t)(where_end - where_start);
            char where_buf[256] = {0};
            if (wlen >= sizeof(where_buf)) wlen = sizeof(where_buf) - 1;
            strncpy(where_buf, where_start, wlen);
            where_buf[wlen] = '\0';
            trim(where_buf);

            // parse "col op value"
            char col[64], op[4], val[128];
            memset(col,0,sizeof(col)); memset(op,0,sizeof(op)); memset(val,0,sizeof(val));
            // Accept-=,>
            if (sscanf(where_buf, "%63s %3s %127[^\n]", col, op, val) == 3) {
                trim(col); trim(op); trim(val);
                if ((val[0] == '\'' && val[strlen(val)-1] == '\'') || (val[0]=='\"' && val[strlen(val)-1]=='\"')) {
                    ctx.where_is_str = 1;
                    size_t l = strlen(val);
                    if (l > 1) {
                        strncpy(ctx.where_str, val+1, sizeof(ctx.where_str)-1);
                        ctx.where_str[sizeof(ctx.where_str)-1] = '\0';
                        if (ctx.where_str[strlen(ctx.where_str)-1] == '\'' || ctx.where_str[strlen(ctx.where_str)-1] == '\"')
                            ctx.where_str[strlen(ctx.where_str)-1] = '\0';
                    }
                } else {
                    ctx.where_is_str = 0;
                    ctx.where_val = atoi(val);
                }
                strncpy(ctx.where_col, col, sizeof(ctx.where_col)-1);
                ctx.where_col[sizeof(ctx.where_col)-1] = '\0';
                strncpy(ctx.where_op, op, sizeof(ctx.where_op)-1);
                ctx.where_op[sizeof(ctx.where_op)-1] = '\0';
                ctx.has_where = 1;
            }
        }
        if (order_ptr) {
            const char *order_start = order_ptr + strlen("ORDER BY");
            char order_buf[128];
            strncpy(order_buf, order_start, sizeof(order_buf)-1);
            order_buf[sizeof(order_buf)-1] = '\0';
            trim(order_buf);
            char ocol[64], odir[8] = {0};
            if (sscanf(order_buf, "%63s %7s", ocol, odir) >= 1) {
                strncpy(ctx.order_col, ocol, sizeof(ctx.order_col)-1);
                ctx.order_col[sizeof(ctx.order_col)-1] = '\0';
                if (odir[0] == '\0') strcpy(ctx.order_dir, "ASC");
                else {
                    strncpy(ctx.order_dir, odir, sizeof(ctx.order_dir)-1);
                    ctx.order_dir[sizeof(ctx.order_dir)-1] = '\0';
                }
                ctx.has_order = 1;
            }
        }

        if (strlen(cols_buf) == 0) {
            ctx.ncols = 0;
        } else {
            if (strcmp(cols_buf, "*") == 0) {
                ctx.ncols = 0;
            } else {
                char tmp[256];
                strncpy(tmp, cols_buf, sizeof(tmp)-1);
                tmp[sizeof(tmp)-1] = '\0';
                char *tok = strtok(tmp, ",");
                while (tok && ctx.ncols < 8) {
                    trim(tok);
                    strncpy(ctx.cols[ctx.ncols], tok, sizeof(ctx.cols[0]) - 1);
                    ctx.cols[ctx.ncols][sizeof(ctx.cols[0]) - 1] = '\0';
                    ctx.ncols++;
                    tok = strtok(NULL, ",");
                }
            }
        }

        ResultSet all = { .count = 0 };
        bptree_for_each(index, collect_record, &all);

        Record *filtered[1024];
        int fcount = 0;

        for (int i = 0; i < all.count; ++i) {
            Record *r = all.records[i];
            int match = 1;
            if (ctx.has_where) {
                match = 0;
                if (strcasecmp(ctx.where_col, "id") == 0 && !ctx.where_is_str) {
                    if (strcmp(ctx.where_op, "=") == 0 && r->id == ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, ">") == 0) && r->id > ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "<") == 0) && r->id < ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, ">=") == 0) && r->id >= ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "<=") == 0) && r->id <= ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "!=") == 0) && r->id != ctx.where_val) match = 1;
                } else if (strcasecmp(ctx.where_col, "marks") == 0 && !ctx.where_is_str) {
                    if (strcmp(ctx.where_op, "=") == 0 && r->marks == ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, ">") == 0) && r->marks > ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "<") == 0) && r->marks < ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, ">=") == 0) && r->marks >= ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "<=") == 0) && r->marks <= ctx.where_val) match = 1;
                    else if ((strcmp(ctx.where_op, "!=") == 0) && r->marks != ctx.where_val) match = 1;
                } else if (strcasecmp(ctx.where_col, "name") == 0 && ctx.where_is_str) {
                    if (strcmp(ctx.where_op, "=") == 0 && strcasecmp(r->name, ctx.where_str) == 0) match = 1;
                    else if (strcmp(ctx.where_op, "!=") == 0 && strcasecmp(r->name, ctx.where_str) != 0) match = 1;
                }
            }
            if (match) filtered[fcount++] = r;
        }

        // ordering
        if (ctx.has_order && fcount > 1) {
            for (int i = 0; i < fcount - 1; ++i) {
                for (int j = i + 1; j < fcount; ++j) {
                    if (compare_record(filtered[i], filtered[j], ctx.order_col, ctx.order_dir) > 0) {
                        Record *tmp = filtered[i];
                        filtered[i] = filtered[j];
                        filtered[j] = tmp;
                    }
                }
            }
        }

        // print header
        if (ctx.ncols == 0) {
            printf("\nID | Name       | Marks\n");
            printf("-------------------------\n");
        } else {
            for (int i = 0; i < ctx.ncols; ++i) {
                printf("%s", ctx.cols[i]);
                if (i + 1 < ctx.ncols) printf(" | ");
            }
            printf("\n");
            for (int i = 0; i < ctx.ncols * 3; ++i) printf("-");
            printf("\n");
        }

        // print rows
        for (int i = 0; i < fcount; ++i) {
            Record *r = filtered[i];
            if (ctx.ncols == 0) {
                printf("%d | %-10s | %d\n", r->id, r->name, r->marks);
            } else {
                for (int j = 0; j < ctx.ncols; ++j) {
                    if (strcasecmp(ctx.cols[j], "id") == 0) printf("%d", r->id);
                    else if (strcasecmp(ctx.cols[j], "name") == 0) printf("%s", r->name);
                    else if (strcasecmp(ctx.cols[j], "marks") == 0) printf("%d", r->marks);
                    else printf("NULL");
                    if (j + 1 < ctx.ncols) printf(" | ");
                }
                printf("\n");
            }
        }
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

    else if (strncasecmp(cmd, "DELETE FROM students WHERE", 25) == 0) {
        const char *where_ptr = strcasestr(cmd, "WHERE");
        if (!where_ptr) {
            printf("Invalid DELETE syntax.\nExample: DELETE FROM students WHERE id = 5;\n");
            return;
        }
        char where_buf[128] = {0};
        strncpy(where_buf, where_ptr + 5, sizeof(where_buf)-1);
        trim(where_buf);
        char col[64], op[4], val[128];
        if (sscanf(where_buf, "%63s %3s %127[^\n]", col, op, val) == 3) {
            trim(col); trim(op); trim(val);
            if (strcasecmp(col, "id") == 0) {
                int id = atoi(val);
                bptree_delete(index, id);
                printf("[B+Tree] Deleted key=%d\n", id);
                printf("Deleted record(s) with ID=%d\n", id);
                return;
            }
        }
        printf("Invalid DELETE syntax.\nExample: DELETE FROM students WHERE id = 5;\n");
        return;
    }

    else if (strncasecmp(cmd, "UPDATE students SET", 19) == 0) {
        char *set_ptr = strcasestr(cmd, "SET");
        char *where_ptr = strcasestr(cmd, "WHERE");
        if (!set_ptr || !where_ptr) {
            printf("Invalid UPDATE syntax.\nExample: UPDATE students SET marks = 90 WHERE id = 2;\n");
            return;
        }
        // extract where
        char where_buf[128] = {0};
        strncpy(where_buf, where_ptr + 5, sizeof(where_buf)-1);
        trim(where_buf);
        int id = -1;
        if (sscanf(where_buf, "id = %d", &id) != 1 && sscanf(where_buf, "ID = %d", &id) != 1) {
            printf("Invalid or missing WHERE clause.\nExample: UPDATE students SET marks = 90 WHERE id = 2;\n");
            return;
        }

        // extract set portion
        const char *set_start = set_ptr + 3;
        size_t set_len = (size_t)(where_ptr - set_start);
        char set_part[256] = {0};
        if (set_len >= sizeof(set_part)) set_len = sizeof(set_part)-1;
        strncpy(set_part, set_start, set_len);
        set_part[set_len] = '\0';
        trim(set_part);

        // parse assign
        char name_val[128] = {0};
        int marks_val = -1;
        char *tok = strtok(set_part, ",");
        while (tok) {
            trim(tok);
            char tmp_name[128], tmp_q[128];
            if (sscanf(tok, "name = '%127[^']'", tmp_name) == 1 ||
                sscanf(tok, "name = \"%127[^\"]\"", tmp_name) == 1) {
                strncpy(name_val, tmp_name, sizeof(name_val)-1);
            } else if (sscanf(tok, "marks = %d", &marks_val) == 1) {
                // marks_val set
            }
            tok = strtok(NULL, ",");
        }

        Record *r = bptree_search(index, id);
        if (!r) {
            printf("No record found with ID=%d\n", id);
            return;
        }

        if (strlen(name_val) > 0) {
            strncpy(r->name, name_val, sizeof(r->name)-1);
            r->name[sizeof(r->name)-1] = '\0';
        }
        if (marks_val >= 0) r->marks = marks_val;

        printf("Updated record ID=%d -> Name=%s, Marks=%d\n", r->id, r->name, r->marks);
        return;
    }

    else if (strncasecmp(cmd, "HELP", 4) == 0) {
        printf("\nAvailable Commands:\n");
        printf("  SELECT <expr>;\n");
        printf("  SELECT * FROM students;\n");
        printf("  SELECT * FROM students WHERE id = n;\n");
        printf("  SELECT * FROM students WHERE name = 'X';\n");
        printf("  SELECT * FROM students WHERE marks > 90 ORDER BY marks DESC;\n");
        printf("  INSERT INTO students VALUES (id, 'name', marks);\n");
        printf("  UPDATE students SET marks=95 WHERE id=n;\n");
        printf("  DELETE FROM students WHERE id=n;\n");
        printf("  HELP;\n");
        printf("  EXIT;\n\n");
        return;
    }

    else if (strncasecmp(cmd, "EXIT", 4) == 0) {
        printf("Exiting MiniDB...\n");
        exit(0);
    }

    printf("Unknown command, Type HELP for help.\n");
}