CC = gcc
CFLAGS = -ldl -Wall
LDFLAGS = -lpthread

all: test_vdso

test_vdso: test_vdso.c
	test -n "$(LINUX_HEADERS_DIR)" || (echo "LINUX_HEADERS_DIR is not set"; exit 1)
	$(CC) $(CFLAGS) -I$(LINUX_HEADERS_DIR) -o test_vdso test_vdso.c

copy: test_vdso
	if ! mountpoint -q $(QEMU_TEST_DIR)/rootfs; then sudo mount -o loop $(QEMU_TEST_DIR)/rootfs.img $(QEMU_TEST_DIR)/rootfs; fi && \
	sudo cp test_vdso $(QEMU_TEST_DIR)/rootfs/tests

clean:
	rm -f test_vdso