#include "sql_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void sql_execute(const char *query, BPTree *index) {
    char cmd[256];
    strcpy(cmd, query);
    trim(cmd);

    if (strlen(cmd) == 0) return;

    if (strncasecmp(cmd, "SELECT", 6) == 0) {
        const char *expr = cmd + 6;
        while (isspace(*expr)) expr++;

        char expr_copy[128];
        strncpy(expr_copy, expr, sizeof(expr_copy) - 1);
        expr_copy[sizeof(expr_copy) - 1] = '\0';

        char *semi = strchr(expr_copy, ';');
        if (semi) *semi = '\0';
        trim(expr_copy);

        int result = evaluate_expression(expr_copy);
        printf("Result: %d\n", result);
    }
    else if (strncasecmp(cmd, "INSERT", 6) == 0) {
        int id, marks;
        char name[50];

        if (sscanf(cmd, "INSERT INTO students VALUES (%d, '%49[^']', %d);", &id, name, &marks) == 3) {
            Record rec;
            rec.id = id;
            strcpy(rec.name, name);
            rec.marks = marks;

            bptree_insert(index, rec.id, &rec);
            printf("Record inserted: ID=%d, Name=%s, Marks=%d\n", rec.id, rec.name, rec.marks);
        } else {
            printf("Invalid INSERT syntax.\n");
            printf("Example: INSERT INTO students VALUES (1, 'Alice', 95);\n");
        }
    }
    else if (strncasecmp(cmd, "HELP", 4) == 0) {
        printf("\nAvailable Commands:\n");
        printf("  SELECT <expr>;               -> Evaluate arithmetic\n");
        printf("  INSERT INTO students VALUES (id, 'name', marks);\n");
        printf("  HELP;                        -> Show help message\n");
        printf("  EXIT;                        -> Quit program\n\n");
    }
    else if (strncasecmp(cmd, "SELECT * FROM students", 22) == 0) {
        printf("\nID | Name     | Marks\n");
        printf("-----------------------\n");
        bptree_traverse(index);
    }
    else if (strncasecmp(cmd, "EXIT", 4) == 0) {
        printf("Exiting MiniDB...\n");
        exit(0);
    }
    else {
        printf("Unknown command. Type HELP for help.\n");
    }
}