#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

// 引入你的实现
extern int get_xattr(const char *path, const char *name, char *value, size_t size, int flags);
extern int set_xattr(const char *path, const char *name, const char *value, size_t size, int flags);
extern int remove_xattr(const char *path, const char *name);
extern ssize_t list_xattr(const char *path, char *list, size_t size);

// 辅助函数：打印错误信息
void print_error(const char *msg) {
    perror(msg);
    printf("错误码: %d\n", errno);
}

// 记录测试结果的结构体
typedef struct {
    int set_result;
    int get_result;
    char get_buffer[100];
    int list_result;
    char list_buffer[1024];
    int remove_result;
    int get_after_remove_result;
} TestResults;

// 完整的测试流程
void run_full_test(const char *path, const char *test_type, 
                  int (*set_fn)(const char*, const char*, const char*, size_t, int),
                  int (*get_fn)(const char*, const char*, char*, size_t, int),
                  ssize_t (*list_fn)(const char*, char*, size_t),
                  int (*remove_fn)(const char*, const char*),
                  TestResults *results) {
    
    const char *attr_name = "user.test_attr";
    const char *attr_value = "Hello, xattr!";
    size_t value_len = strlen(attr_value);
    
    printf("\n=== 测试 %s 实现 (文件: %s) ===\n", test_type, path);
    
    // 1. 设置属性
    results->set_result = set_fn(path, attr_name, attr_value, value_len, 0);
    printf("设置属性: %s (返回值: %d)\n", 
           results->set_result == 0 ? "成功" : "失败", results->set_result);
    
    // 2. 获取属性
    memset(results->get_buffer, 0, sizeof(results->get_buffer));
    results->get_result = get_fn(path, attr_name, results->get_buffer, sizeof(results->get_buffer), 0);
    printf("获取属性: %s (返回值: %d, 值: \"%s\")\n", 
           results->get_result >= 0 ? "成功" : "失败", results->get_result, 
           results->get_result >= 0 ? results->get_buffer : "");
    
    // 3. 列出所有属性
    memset(results->list_buffer, 0, sizeof(results->list_buffer));
    results->list_result = list_fn(path, results->list_buffer, sizeof(results->list_buffer));
    printf("列出属性: %s (返回值: %d)\n", 
           results->list_result >= 0 ? "成功" : "失败", results->list_result);
    
    if (results->list_result > 0) {
        printf("属性列表: ");
        const char *attr = results->list_buffer;
        while (attr < results->list_buffer + results->list_result) {
            printf("%s ", attr);
            attr += strlen(attr) + 1;
        }
        printf("\n");
    }
    
    // 4. 删除属性
    results->remove_result = remove_fn(path, attr_name);
    printf("删除属性: %s (返回值: %d)\n", 
           results->remove_result == 0 ? "成功" : "失败", results->remove_result);
    
    // 5. 尝试获取已删除属性
    results->get_after_remove_result = get_fn(path, attr_name, results->get_buffer, sizeof(results->get_buffer), 0);
    printf("获取已删除属性: %s (返回值: %d)\n", 
           results->get_after_remove_result < 0 ? "正确返回错误" : "错误", results->get_after_remove_result);
}

// 比较测试结果
void compare_results(TestResults *custom, TestResults *standard) {
    printf("\n=== 结果比较 ===\n");
    
    printf("设置属性: %s (自定义: %d, 标准库: %d)\n",
           custom->set_result == standard->set_result ? "相同" : "不同",
           custom->set_result, standard->set_result);
           
    printf("获取属性: %s (自定义: %d, 标准库: %d)\n",
           custom->get_result == standard->get_result ? "相同" : "不同",
           custom->get_result, standard->get_result);
           
    printf("获取内容: %s\n",
           strcmp(custom->get_buffer, standard->get_buffer) == 0 ? 
           "相同" : "不同");
           
    printf("列出属性大小: %s (自定义: %d, 标准库: %d)\n",
           custom->list_result == standard->list_result ? "相同" : "不同",
           custom->list_result, standard->list_result);
           
    printf("列出内容: %s\n",
           memcmp(custom->list_buffer, standard->list_buffer, custom->list_result) == 0 ? 
           "相同" : "不同");
           
    printf("删除属性: %s (自定义: %d, 标准库: %d)\n",
           custom->remove_result == standard->remove_result ? "相同" : "不同",
           custom->remove_result, standard->remove_result);
           
    printf("获取已删除属性: %s (自定义: %d, 标准库: %d)\n",
           custom->get_after_remove_result == standard->get_after_remove_result ? "相同" : "不同",
           custom->get_after_remove_result, standard->get_after_remove_result);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("用法: %s <测试文件路径>\n", argv[0]);
        return 1;
    }
    
    const char *path = argv[1];
    TestResults custom_results = {0};
    TestResults standard_results = {0};
    
    // 创建测试文件
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        print_error("无法创建测试文件");
        return 1;
    }
    close(fd);
    
    // 首先用自定义实现测试
    run_full_test(path, "自定义", set_xattr, get_xattr, list_xattr, remove_xattr, &custom_results);
    
    // 清理文件（重新创建）
    unlink(path);
    fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        print_error("无法重新创建测试文件");
        return 1;
    }
    close(fd);
    
    // 然后用标准库实现测试
    run_full_test(path, "标准库",  (int (*)(const char*, const char*, const char*, size_t, int))setxattr, 
             (int (*)(const char*, const char*, char*, size_t, int))getxattr, listxattr, removexattr, &standard_results);

    // 比较结果
    compare_results(&custom_results, &standard_results);

    printf("\n=== 测试完成 ===\n");

    return 0;
}