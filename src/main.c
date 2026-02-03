#include "csv.h"
#include "query.h"
#include <stdio.h>
#include <string.h>

static int find_col(const CsvHeader *hdr, const char *name);

static int print_selected(const CsvHeader *hdr, const CsvRow *row, void *user) {
    Query *q = (Query *)user;

    for (size_t i = 0; i < q->nselect; i++) {
        int idx = find_col(hdr, q->select_cols[i]);
        if (idx >= 0 && (size_t)idx < row->nfields) {
            printf("%s", row->fields[idx]);
        }
        if (i + 1 < q->nselect) printf(",");
    }
    printf("\n");
    return 0;
}

static int find_col(const CsvHeader *hdr, const char *name) {
    for (size_t i = 0; i < hdr->ncols; i++) {
        if (strcmp(hdr->cols[i], name) == 0)
            return (int)i;
    }
    return -1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"SELECT col1,col2 FROM file.csv\"\n", argv[0]);
        fprintf(stderr, "Example: %s \"SELECT name,city FROM data/people.csv\"\n", argv[0]);
        return 1;
    }

    Query q;
    if (parse_query(argv[1], &q) != 0) {
        fprintf(stderr, "Invalid query\n");
        return 1;
    }

    const char *path = q.from_path;

    CsvHeader hdr;
    if (csv_read_header(path, &hdr) != 0) {
        fprintf(stderr, "Failed to read CSV header from %s\n", path);
        free_query(&q);
        return 1;
    }

    if (csv_read_rows(path, &hdr, print_selected, &q) != 0) {
        fprintf(stderr, "Failed to read rows from %s\n", path);
        csv_free_header(&hdr);
        free_query(&q);
        return 1;
    }

    csv_free_header(&hdr);
    free_query(&q);
    return 0;
}
