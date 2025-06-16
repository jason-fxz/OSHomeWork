CC = gcc
CFLAGS = -Wall -Wextra
TARGET = test
SOURCE = test.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
