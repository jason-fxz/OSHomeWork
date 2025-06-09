# Socket Fairness 测试程序 Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
TARGET = socket_fair_test
SOURCE = socket_fair_test.c

.PHONY: all clean test help

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

test: $(TARGET)
	@echo "运行 Socket Fairness 测试..."
	@./$(TARGET)

clean:
	rm -f $(TARGET)

help:
	@echo "可用目标:"
	@echo "  all     - 编译测试程序"
	@echo "  test    - 编译并运行测试"
	@echo "  clean   - 清理编译文件"
	@echo "  help    - 显示此帮助信息"
