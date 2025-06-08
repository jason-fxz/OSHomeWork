CC = gcc
CFLAGS = -Wall -Wextra -O2 -static
LDFLAGS = -lpthread

# 目标文件
TARGETS = test_kv test_basic test_concurrent test1-serial kv.so

# 源文件
SOURCES = test_kv.c test_basic.c test_concurrent.c test1-serial.c kv_lua_binding.c

# 默认目标
all: $(TARGETS)

# 基本测试程序
test_kv: test_kv.c kv_syscalls.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 基础功能测试
test_basic: test_basic.c kv_syscalls.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 并发测试程序
test_concurrent: test_concurrent.c kv_syscalls.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 串行一致性测试
test1-serial: test1-serial.c kv_syscalls.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 生成共享库
kv.so: kv_lua_binding.c
	$(CC) -shared -fPIC -o $@ $< -llua

# 清理目标文件
clean:
	rm -f $(TARGETS)

# 重新编译
rebuild: clean all

.PHONY: all clean rebuild