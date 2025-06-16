CC = g++
CFLAGS = -Wall -Wextra
TARGET = test_vdso_all test_vdso

.PHONY: all clean

all: $(TARGET)

test_vdso_all: test_vdso_all.cpp
	$(CC) $(CFLAGS) -o $@ $<

test_vdso: test_vdso.cpp
	$(CC) $(CFLAGS) -o $@ $<
	
clean:
	rm -f $(TARGET)
