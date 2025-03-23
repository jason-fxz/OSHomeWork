// 使用示例
#include "kv_syscalls.h"
#include <stdio.h>

int main() {
    int ret;
    
    // 写入键值对
    ret = write_kv(1, 100);
    if (ret < 0) {
        perror("write_kv failed");
        return 1;
    }
    
    // 读取键值对
    ret = read_kv(1);
    if (ret < 0) {
        perror("read_kv failed");
        return 1;
    }
    
    printf("Value: %d\n", ret);
    return 0;
}