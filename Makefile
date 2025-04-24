QEMU_TEST_DIR = $(PWD)/qemu_test

QEMU_NCPU ?= 4

LINUX_HEADERS_DIR = $(PWD)/headers/include

qemu: kv_store vdso
	qemu-system-x86_64 \
		-kernel linux-5.15.178/arch/x86/boot/bzImage \
		-drive format=raw,file=qemu_test/rootfs.img \
		-append "root=/dev/sda console=ttyS0 nokaslr" \
		-nographic \
		-enable-kvm \
		-smp $(QEMU_NCPU) \
		-m 4G

kv_store:
	make -C syscall/kv_syscall copy QEMU_TEST_DIR=$(QEMU_TEST_DIR)

vdso:
	make -C syscall/vdso copy QEMU_TEST_DIR=$(QEMU_TEST_DIR) LINUX_HEADERS_DIR=$(LINUX_HEADERS_DIR)

kernel:
	make -C linux-5.15.178 -j$(shell nproc) && \
	make -C linux-5.15.178 headers_install INSTALL_HDR_PATH=$(LINUX_HEADERS_DIR)