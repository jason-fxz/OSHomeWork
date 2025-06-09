/*
 * Socket Fairness 测试程序
 * 
 * 功能：
 * 1. 测试系统调用是否正常工作
 * 2. 测试套接字限制功能
 * 3. 验证错误处理
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

/* 系统调用号 */
#define SYS_configure_socket_fairness 449

/* 配置套接字公平性的系统调用 */
long configure_socket_fairness(pid_t thread_id, int max_socket_allowed)
{
    return syscall(SYS_configure_socket_fairness, thread_id, max_socket_allowed);
}

/* 获取线程ID */
pid_t gettid(void)
{
    return syscall(SYS_gettid);
}

void print_test_header(const char *test_name)
{
    printf("\n=== %s ===\n", test_name);
}

int test_basic_syscall(void)
{
    pid_t tid = gettid();
    long result;
    
    print_test_header("基本系统调用测试");
    printf("当前线程 TID: %d\n", tid);
    
    /* 测试设置限制为 5 */
    result = configure_socket_fairness(tid, 5);
    if (result == 0) {
        printf("✓ 设置套接字限制为 5 成功\n");
        return 0;
    } else {
        printf("✗ 设置套接字限制失败: %s (错误码: %ld)\n", strerror(-result), result);
        return -1;
    }
}

int test_socket_limits(void)
{
    pid_t tid = gettid();
    int sockets[10];
    int i, created = 0;
    long result;
    int limit = 3;
    
    print_test_header("套接字限制测试");
    
    /* 设置限制为 3 */
    result = configure_socket_fairness(tid, limit);
    if (result != 0) {
        printf("✗ 无法设置套接字限制: %s\n", strerror(-result));
        return -1;
    }
    printf("设置套接字限制为: %d\n", limit);
    
    /* 尝试创建超过限制数量的套接字 */
    printf("尝试创建 6 个套接字...\n");
    for (i = 0; i < 6; i++) {
        sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockets[i] >= 0) {
            created++;
            printf("  套接字 %d: 创建成功 (fd=%d)\n", i+1, sockets[i]);
        } else {
            printf("  套接字 %d: 创建失败 - %s\n", i+1, strerror(errno));
            break;
        }
    }
    
    /* 检查结果 */
    printf("\n测试结果: 成功创建 %d 个套接字\n", created);
    if (created <= limit) {
        printf("✓ 套接字限制正常工作 (限制: %d, 创建: %d)\n", limit, created);
    } else {
        printf("✗ 套接字限制未生效 (限制: %d, 创建: %d)\n", limit, created);
    }
    
    /* 清理资源 */
    for (i = 0; i < created; i++) {
        if (sockets[i] >= 0) {
            close(sockets[i]);
        }
    }
    
    return (created <= limit) ? 0 : -1;
}

int test_error_handling(void)
{
    pid_t tid = gettid();
    long result;
    
    print_test_header("错误处理测试");
    
    /* 测试负数限制 */
    result = configure_socket_fairness(tid, -1);
    if (result < 0) {
        printf("✓ 正确拒绝负数限制\n");
    } else {
        printf("✗ 应该拒绝负数限制 (返回值: %ld)\n", result);
        return -1;
    }
    
    /* 测试过大的限制 */
    result = configure_socket_fairness(tid, 100000);
    if (result < 0) {
        printf("✓ 正确拒绝过大限制\n");
    } else {
        printf("✗ 应该拒绝过大限制 (返回值: %ld)\n", result);
        return -1;
    }
    
    /* 测试无效的 PID */
    result = configure_socket_fairness(99999, 10);
    if (result < 0) {
        printf("✓ 正确拒绝无效PID\n");
    } else {
        printf("✗ 应该拒绝无效PID (返回值: %ld)\n", result);
        return -1;
    }
    
    return 0;
}

int test_reset_limits(void)
{
    pid_t tid = gettid();
    int sockets[8];
    int i, created_before = 0, created_after = 0;
    long result;
    
    print_test_header("限制重置测试");
    
    /* 先设置限制为 2 */
    result = configure_socket_fairness(tid, 2);
    if (result != 0) {
        printf("✗ 无法设置初始限制\n");
        return -1;
    }
    printf("设置初始限制为: 2\n");
    
    /* 创建套接字测试 */
    for (i = 0; i < 4; i++) {
        sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockets[i] >= 0) {
            created_before++;
        } else {
            break;
        }
    }
    printf("在限制为2时创建了 %d 个套接字\n", created_before);
    
    /* 关闭套接字 */
    for (i = 0; i < created_before; i++) {
        close(sockets[i]);
    }
    
    /* 重新设置限制为 5 */
    result = configure_socket_fairness(tid, 5);
    if (result != 0) {
        printf("✗ 无法重置限制\n");
        return -1;
    }
    printf("重置限制为: 5\n");
    
    /* 再次创建套接字测试 */
    for (i = 0; i < 7; i++) {
        sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockets[i] >= 0) {
            created_after++;
        } else {
            break;
        }
    }
    printf("在限制为5时创建了 %d 个套接字\n", created_after);
    
    /* 清理资源 */
    for (i = 0; i < created_after; i++) {
        close(sockets[i]);
    }
    
    /* 验证结果 */
    if (created_before <= 2 && created_after <= 5 && created_after > created_before) {
        printf("✓ 限制重置功能正常\n");
        return 0;
    } else {
        printf("✗ 限制重置功能异常\n");
        return -1;
    }
}

int main(void)
{
    int total_tests = 0, passed_tests = 0;
    
    printf("Socket Fairness 测试程序\n");
    printf("========================\n");
    
    /* 运行所有测试 */
    total_tests++;
    if (test_basic_syscall() == 0) passed_tests++;
    
    total_tests++;
    if (test_socket_limits() == 0) passed_tests++;
    
    total_tests++;
    if (test_error_handling() == 0) passed_tests++;
    
    total_tests++;
    if (test_reset_limits() == 0) passed_tests++;
    
    /* 显示总结 */
    printf("\n=== 测试总结 ===\n");
    printf("总测试数: %d\n", total_tests);
    printf("通过测试: %d\n", passed_tests);
    printf("失败测试: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("✓ 所有测试通过!\n");
        return 0;
    } else {
        printf("✗ 有测试失败\n");
        return 1;
    }
}
