#include "csv.h"
#include <stdio.h>

static int print_row(const CsvHeader *hdr, const CsvRow *row, void *user) {
    (void)user;
    size_t n = hdr->ncols < row->nfields ? hdr->ncols : row->nfields;
    for (size_t i = 0; i < n; i++) {
        printf("%s%s", row->fields[i], (i + 1 == n ? "" : ","));
    }
    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <csv_file>\n", argv[0]);
        fprintf(stderr, "Example: %s data/people.csv\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];

    CsvHeader hdr;
    if (csv_read_header(path, &hdr) != 0) {
        fprintf(stderr, "Failed to read CSV header from %s\n", path);
        return 1;
    }

    // print header
    for (size_t i = 0; i < hdr.ncols; i++) {
        printf("%s%s", hdr.cols[i], (i + 1 == hdr.ncols ? "" : ","));
    }
    printf("\n");

    if (csv_read_rows(path, &hdr, print_row, NULL) != 0) {
        fprintf(stderr, "Failed to read rows from %s\n", path);
        csv_free_header(&hdr);
        return 1;
    }

    csv_free_header(&hdr);
    return 0;
}
