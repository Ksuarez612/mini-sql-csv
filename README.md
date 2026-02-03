# mini-sql-csv

A small C program that runs simple SQL-style queries against CSV files.

It processes CSV data row by row and supports a limited subset of SQL features for selecting, filtering, and limiting results.

---

## Features

- `SELECT` columns or `SELECT *`
- `WHERE` filtering  
  - Numeric: `= != < <= > >=`  
  - String: `=` `!=`
- `LIMIT` rows returned
- Clear errors for invalid queries

---

## Example CSV

```csv
name,age,city
Alice,21,NY
Bob,19,NJ
Carol,25,NY
Dan,22,CA
```
