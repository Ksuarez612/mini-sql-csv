# mini-sql-csv

A small C project that reads CSV files and supports basic SQL-style queries.
Built to practice C fundamentals like file I/O, string parsing, and manual memory management.

## Example
```bash
./csql "SELECT name, age FROM data/people.csv WHERE age >= 21"