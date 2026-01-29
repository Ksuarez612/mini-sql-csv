#ifndef CSV_H
#define CSV_H

#include <stddef.h>

typedef struct {
    char **cols;
    size_t ncols;
} CsvHeader;

typedef struct {
    char **fields;
    size_t nfields;
} CsvRow;

int csv_read_header(const char *path, CsvHeader *out);

int csv_read_rows(const char *path, const CsvHeader *hdr,
                  int (*cb)(const CsvHeader*, const CsvRow*, void*),
                  void *user);

void csv_free_header(CsvHeader *hdr);
void csv_free_row(CsvRow *row);

#endif
