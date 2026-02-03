#include "csv.h"
#include "query.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int find_col(const CsvHeader *hdr, const char *name) {
    for (size_t i = 0; i < hdr->ncols; i++) {
        if (strcmp(hdr->cols[i], name) == 0)
            return (int)i;
    }
    return -1;
}

static int parse_long(const char *s, long *out) {
    if (!s || !*s) return 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0') return 0;
    *out = v;
    return 1;
}

typedef struct {
    Query *q;
    int *sel_idx;
    size_t nsel;
    int where_idx;        // column index for WHERE, or -1
    long printed;         // how many rows printed so far
} Runtime;

static int where_matches(const Query *q, const CsvRow *row, int where_idx) {
    if (!q->has_where) return 1;
    if (where_idx < 0 || (size_t)where_idx >= row->nfields) return 0;

    const char *field = row->fields[where_idx];

    // Numeric comparison
    if (q->where_is_num) {
        long fv = 0;
        if (!parse_long(field, &fv)) {
            fprintf(stderr,
                "Type error: WHERE column '%s' is not numeric\n",
                q->where_col);
            return -1;
        }

        switch (q->where_op) {
            case OP_EQ:  return fv == q->where_num;
            case OP_NEQ: return fv != q->where_num;
            case OP_LT:  return fv <  q->where_num;
            case OP_LTE: return fv <= q->where_num;
            case OP_GT:  return fv >  q->where_num;
            case OP_GTE: return fv >= q->where_num;
            default: return -1;
        }
    }

    // String comparison
    int cmp = strcmp(field, q->where_val);

    if (q->where_op != OP_EQ && q->where_op != OP_NEQ) {
        fprintf(stderr,
            "Invalid operator for string column '%s'\n",
            q->where_col);
        return -1;
    }

    return (q->where_op == OP_EQ) ? (cmp == 0) : (cmp != 0);
}
static int print_selected(const CsvHeader *hdr, const CsvRow *row, void *user) {
    (void)hdr;
    Runtime *rt = (Runtime *)user;
    Query *q = rt->q;

    int wm = where_matches(q, row, rt->where_idx);
    if (wm < 0) return 1;   // hard error -> stop reading
    if (wm == 0) return 0;  // no match -> skip row

    // LIMIT check (stop early)
    if (q->has_limit && rt->printed >= q->limit) {
        return 1;
    }

    for (size_t i = 0; i < rt->nsel; i++) {
        int idx = rt->sel_idx[i];
        if (idx >= 0 && (size_t)idx < row->nfields) {
            printf("%s", row->fields[idx]);
        }
        if (i + 1 < rt->nsel) printf(",");
    }
    printf("\n");

    rt->printed++;

    if (q->has_limit && rt->printed >= q->limit) {
        return 1;
    }

    return 0;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"SELECT ... FROM file.csv [WHERE ...] [LIMIT N]\"\n", argv[0]);
        fprintf(stderr, "Example: %s \"SELECT name,city FROM data/people.csv WHERE age >= 21 LIMIT 2\"\n", argv[0]);
        return 1;
    }

    Query q;
    if (parse_query(argv[1], &q) != 0) {
        fprintf(stderr, "Invalid query\n");
        return 1;
    }

    CsvHeader hdr;
    if (csv_read_header(q.from_path, &hdr) != 0) {
        fprintf(stderr, "Failed to read CSV header from %s\n", q.from_path);
        free_query(&q);
        return 1;
    }

    // Build selected column indices once (faster + cleaner)
    int *sel_idx = NULL;
    size_t nsel = 0;

    if (q.select_all) {
        nsel = hdr.ncols;
        sel_idx = malloc(nsel * sizeof(int));
        if (!sel_idx) {
            csv_free_header(&hdr);
            free_query(&q);
            return 1;
        }
        for (size_t i = 0; i < nsel; i++) sel_idx[i] = (int)i;
    } else {
        nsel = q.nselect;
        sel_idx = malloc(nsel * sizeof(int));
        if (!sel_idx) {
            csv_free_header(&hdr);
            free_query(&q);
            return 1;
        }
        for (size_t i = 0; i < nsel; i++) {
            sel_idx[i] = find_col(&hdr, q.select_cols[i]);
            if (sel_idx[i] < 0) {
                fprintf(stderr, "Unknown column in SELECT: %s\n", q.select_cols[i]);
                free(sel_idx);
                csv_free_header(&hdr);
                free_query(&q);
                return 1;
            }
        }
    }

    int where_idx = -1;
    if (q.has_where) {
        where_idx = find_col(&hdr, q.where_col);
        if (where_idx < 0) {
            fprintf(stderr, "Unknown column in WHERE: %s\n", q.where_col);
            free(sel_idx);
            csv_free_header(&hdr);
            free_query(&q);
            return 1;
        }
    }

    Runtime rt = {0};
    rt.q = &q;
    rt.sel_idx = sel_idx;
    rt.nsel = nsel;
    rt.where_idx = where_idx;
    rt.printed = 0;

    if (csv_read_rows(q.from_path, &hdr, print_selected, &rt) != 0) {
        fprintf(stderr, "Failed to read rows from %s\n", q.from_path);
        free(sel_idx);
        csv_free_header(&hdr);
        free_query(&q);
        return 1;
    }

    free(sel_idx);
    csv_free_header(&hdr);
    free_query(&q);
    return 0;
}
