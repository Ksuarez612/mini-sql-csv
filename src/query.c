#include "query.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *dup_range(const char *a, const char *b) {
    size_t n = (size_t)(b - a);
    char *s = malloc(n + 1);
    if (!s) return NULL;
    memcpy(s, a, n);
    s[n] = '\0';
    return s;
}

static const char *skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static int match_kw(const char *p, const char *kw) {
    while (*kw) {
        if (tolower((unsigned char)*p) != tolower((unsigned char)*kw))
            return 0;
        p++; kw++;
    }
    return 1;
}

// Supports: SELECT col1,col2 FROM file.csv
int parse_query(const char *input, Query *out) {
    memset(out, 0, sizeof(*out));

    const char *p = skip_ws(input);
    if (!match_kw(p, "select")) return -1;
    p += 6;

    size_t cap = 4;
    out->select_cols = malloc(cap * sizeof(char*));
    if (!out->select_cols) return -1;

    while (1) {
        p = skip_ws(p);
        if (match_kw(p, "from")) break;

        const char *start = p;
        while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
        if (p == start) return -1;

        if (out->nselect == cap) {
            cap *= 2;
            char **tmp = realloc(out->select_cols, cap * sizeof(char*));
            if (!tmp) return -1;
            out->select_cols = tmp;
        }

        out->select_cols[out->nselect++] = dup_range(start, p);

        p = skip_ws(p);
        if (*p == ',') p++;
    }

    p += 4; // skip FROM
    p = skip_ws(p);

    const char *start = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    out->from_path = dup_range(start, p);

    return 0;
}

void free_query(Query *q) {
    for (size_t i = 0; i < q->nselect; i++)
        free(q->select_cols[i]);
    free(q->select_cols);
    free(q->from_path);
}
