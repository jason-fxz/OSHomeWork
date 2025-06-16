CC = gcc
CFLAGS = -Wall -Wextra

all: test1 test2 test3

test3: test3.c
	$(CC) $(CFLAGS) -o test3 test3.c

test2: test2.c
	$(CC) $(CFLAGS) -o test2 test2.c -lpthread

test1:
	$(CC) $(CFLAGS) -o test1 test1.c

clean:
	rm -f test1 test2 test3

.PHONY: all clean

