CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude -g
SRC = src/main.c src/tun.c
OBJ = $(SRC:.c=.o)
TARGET = tcp_stack

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
