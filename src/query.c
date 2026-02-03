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
        if (tolower((unsigned char)*p) != tolower((unsigned char)*kw)) return 0;
        p++; kw++;
    }
    // ensure keyword boundary (space/end)
    if (*p && !isspace((unsigned char)*p)) return 0;
    return 1;
}

static int parse_ident(const char **pp, char **out) {
    const char *p = skip_ws(*pp);
    const char *start = p;
    while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
    if (p == start) return -1;
    *out = dup_range(start, p);
    if (!*out) return -1;
    *pp = p;
    return 0;
}

static int parse_number(const char *s, long *out) {
    if (!s || !*s) return 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return 0;
    *out = v;
    return 1;
}

static int parse_value(const char **pp, char **out) {
    const char *p = skip_ws(*pp);
    if (*p == '"') {
        p++; // skip opening quote
        const char *start = p;
        while (*p && *p != '"') p++;
        if (*p != '"') return -1; // missing closing quote
        *out = dup_range(start, p);
        if (!*out) return -1;
        p++; // skip closing quote
        *pp = p;
        return 0;
    } else {
        // read until whitespace
        const char *start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (p == start) return -1;
        *out = dup_range(start, p);
        if (!*out) return -1;
        *pp = p;
        return 0;
    }
}

static int parse_op(const char **pp, WhereOp *op) {
    const char *p = skip_ws(*pp);

    if (p[0] == '!' && p[1] == '=') { *op = OP_NEQ; p += 2; }
    else if (p[0] == '=' )          { *op = OP_EQ;  p += 1; }
    else if (p[0] == '<' && p[1] == '=') { *op = OP_LTE; p += 2; }
    else if (p[0] == '>' && p[1] == '=') { *op = OP_GTE; p += 2; }
    else if (p[0] == '<')           { *op = OP_LT;  p += 1; }
    else if (p[0] == '>')           { *op = OP_GT;  p += 1; }
    else return -1;

    *pp = p;
    return 0;
}

int parse_query(const char *input, Query *out) {
    memset(out, 0, sizeof(*out));

    const char *p = skip_ws(input);

    // SELECT
    if (!match_kw(p, "select")) return -1;
    p += 6;
    p = skip_ws(p);

    if (*p == '*') {
        out->select_all = 1;
        p++;
    } else {
        size_t cap = 4;
        out->select_cols = malloc(cap * sizeof(char*));
        if (!out->select_cols) return -1;

        while (1) {
            p = skip_ws(p);
            if (match_kw(p, "from")) break;

            char *col = NULL;
            if (parse_ident(&p, &col) != 0) return -1;

            if (out->nselect == cap) {
                cap *= 2;
                char **tmp = realloc(out->select_cols, cap * sizeof(char*));
                if (!tmp) return -1;
                out->select_cols = tmp;
            }
            out->select_cols[out->nselect++] = col;

            p = skip_ws(p);
            if (*p == ',') p++; // consume comma
        }
    }

    // FROM
    p = skip_ws(p);
    if (!match_kw(p, "from")) return -1;
    p += 4;
    p = skip_ws(p);

    // path until whitespace
    const char *path_start = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    if (p == path_start) return -1;
    out->from_path = dup_range(path_start, p);
    if (!out->from_path) return -1;

    // Optional WHERE and/or LIMIT
    while (1) {
        p = skip_ws(p);
        if (!*p) break;

        if (match_kw(p, "where")) {
            if (out->has_where) return -1; // only one WHERE supported
            p += 5;

            out->has_where = 1;

            if (parse_ident(&p, &out->where_col) != 0) return -1;
            if (parse_op(&p, &out->where_op) != 0) return -1;
            if (parse_value(&p, &out->where_val) != 0) return -1;

            long num = 0;
            if (parse_number(out->where_val, &num)) {
                out->where_is_num = 1;
                out->where_num = num;
            }
            continue;
        }

        if (match_kw(p, "limit")) {
            if (out->has_limit) return -1;
            p += 5;
            p = skip_ws(p);

            // read number token
            const char *start = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (p == start) return -1;

            char *tok = dup_range(start, p);
            if (!tok) return -1;

            long lim = 0;
            int ok = parse_number(tok, &lim);
            free(tok);
            if (!ok || lim < 0) return -1;

            out->has_limit = 1;
            out->limit = lim;
            continue;
        }

        // unknown trailing token
        return -1;
    }

    return 0;
}

void free_query(Query *q) {
    if (!q) return;
    for (size_t i = 0; i < q->nselect; i++) free(q->select_cols[i]);
    free(q->select_cols);
    free(q->from_path);
    free(q->where_col);
    free(q->where_val);
    memset(q, 0, sizeof(*q));
}
