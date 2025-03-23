CC = gcc
CFLAGS = -static
LDFLAGS = -lpthread

all: test_basic test_concurrent

test_basic: test_basic.c
	$(CC) $(CFLAGS) -o test_basic test_basic.c

test_concurrent: test_concurrent.c
	$(CC) $(CFLAGS) -o test_concurrent test_concurrent.c $(LDFLAGS)

copy: test_basic test_concurrent
	if ! mountpoint -q $(QEMU_TEST_DIR)/rootfs; then sudo mount -o loop $(QEMU_TEST_DIR)/rootfs.img $(QEMU_TEST_DIR)/rootfs; fi && \
	sudo cp test_basic test_concurrent $(QEMU_TEST_DIR)/rootfs/tests

clean:
	rm -f test_basic test_concurrent