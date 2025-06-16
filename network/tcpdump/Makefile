CC = gcc
CFLAGS = -Wall -Wextra -lpcap
TARGET = custom_tcpdump
SOURCE = custom_tcpdump.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
