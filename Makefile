CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -Iinclude -g

TARGET = csql
SRC = src/main.c src/csv.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)

asan:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -o $(TARGET) $(SRC)

.PHONY: all clean asan
