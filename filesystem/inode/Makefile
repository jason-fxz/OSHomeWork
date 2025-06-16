CC = gcc
CFLAGS = -Wall -Wextra
TARGET = test
SOURCE = test.c impl.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

run: $(TARGET)
	touch a.txt
	./test a.txt

clean:
	rm -f $(TARGET) a.txt
