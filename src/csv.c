#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

// Very simple split: commas only (no quoted commas yet)
static int split_commas(const char *line_in, char ***out_fields, size_t *out_n) {
    char *line = strdup(line_in);
    if (!line) return -1;

    size_t cap = 8, n = 0;
    char **fields = malloc(cap * sizeof(char*));
    if (!fields) { free(line); return -1; }

    char *save = NULL;
    char *tok = strtok_r(line, ",", &save);

    while (tok) {
        if (n == cap) {
            cap *= 2;
            char **tmp = realloc(fields, cap * sizeof(char*));
            if (!tmp) {
                for (size_t i = 0; i < n; i++) free(fields[i]);
                free(fields);
                free(line);
                return -1;
            }
            fields = tmp;
        }

        fields[n] = strdup(tok);
        if (!fields[n]) {
            for (size_t i = 0; i < n; i++) free(fields[i]);
            free(fields);
            free(line);
            return -1;
        }

        n++;
        tok = strtok_r(NULL, ",", &save);
    }

    free(line);
    *out_fields = fields;
    *out_n = n;
    return 0;
}

int csv_read_header(const char *path, CsvHeader *out) {
    memset(out, 0, sizeof(*out));

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[4096];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    trim_newline(buf);

    char **cols = NULL;
    size_t ncols = 0;
    if (split_commas(buf, &cols, &ncols) != 0) return -1;

    out->cols = cols;
    out->ncols = ncols;
    return 0;
}

int csv_read_rows(const char *path, const CsvHeader *hdr,
                  int (*cb)(const CsvHeader*, const CsvRow*, void*),
                  void *user) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[4096];

    // skip header line
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }

    while (fgets(buf, sizeof(buf), f)) {
        trim_newline(buf);

        CsvRow row;
        memset(&row, 0, sizeof(row));

        if (split_commas(buf, &row.fields, &row.nfields) != 0) {
            fclose(f);
            return -1;
        }

        int rc = cb(hdr, &row, user);
        csv_free_row(&row);

        if (rc != 0) break;
    }

    fclose(f);
    return 0;
}

void csv_free_header(CsvHeader *hdr) {
    if (!hdr || !hdr->cols) return;
    for (size_t i = 0; i < hdr->ncols; i++) free(hdr->cols[i]);
    free(hdr->cols);
    hdr->cols = NULL;
    hdr->ncols = 0;
}

void csv_free_row(CsvRow *row) {
    if (!row || !row->fields) return;
    for (size_t i = 0; i < row->nfields; i++) free(row->fields[i]);
    free(row->fields);
    row->fields = NULL;
    row->nfields = 0;
}
