mini-sql-csv

This project is a small C program that lets you run simple, SQL-style queries against CSV files from the command line. I built it to get more comfortable working in C, especially with pointers, memory management, and parsing legacy-style data formats.

The program reads CSV files row by row (it doesn’t load everything into memory) and supports a subset of SQL features like selecting columns, filtering rows, and limiting results.

What it supports

Reading and parsing CSV files in C

SELECT specific columns or SELECT *

WHERE filtering with basic comparison operators

Numeric: = != < <= > >=

String: = and !=

LIMIT to cap the number of rows returned

Clear error messages for invalid queries or type mismatches

This is not a full SQL engine — the goal was to keep the logic simple and readable while focusing on core C concepts.

Example CSV
name,age,city
Alice,21,NY
Bob,19,NJ
Carol,25,NY
Dan,22,CA

Example usage
# Select specific columns
./csql "SELECT name,city FROM data/people.csv"

# Filter rows with WHERE
./csql "SELECT name,city FROM data/people.csv WHERE age >= 21"

# String comparison
./csql "SELECT name FROM data/people.csv WHERE city = NY"

# Limit results
./csql "SELECT * FROM data/people.csv LIMIT 2"

# Combine WHERE and LIMIT
./csql "SELECT name,city FROM data/people.csv WHERE city = NY LIMIT 1"

Building the project

You’ll need a C compiler (clang or gcc).

make clean
make


This produces the csql executable.

Limitations

No quoted CSV fields

Only one WHERE condition

No AND / OR

No joins or ordering

These were intentional tradeoffs to keep the focus on correctness and clarity rather than feature count.

query.c parses the SQL-style query string

main.c ties everything together and applies filtering/limits
