#ifndef QUERY_H
#define QUERY_H

#include <stddef.h>

typedef struct {
    char **select_cols;   // names from SELECT
    size_t nselect;       // how many columns
    char *from_path;      // csv file path
} Query;

int parse_query(const char *input, Query *out);
void free_query(Query *q);

#endif