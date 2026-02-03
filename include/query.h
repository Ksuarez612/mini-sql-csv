#ifndef QUERY_H
#define QUERY_H

#include <stddef.h>

typedef enum {
    OP_NONE = 0,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE
} WhereOp;

typedef struct {
    // SELECT
    char **select_cols;
    size_t nselect;
    int select_all;

    // FROM
    char *from_path;

    // WHERE (optional)
    int has_where;
    char *where_col;
    WhereOp where_op;
    char *where_val;   // raw value as string (unquoted)
    int where_is_num;
    long where_num;

    // LIMIT (optional)
    int has_limit;
    long limit;
} Query;

int parse_query(const char *input, Query *out);
void free_query(Query *q);

#endif
