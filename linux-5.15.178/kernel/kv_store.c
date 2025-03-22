/**
 * KV_STORE syscalls
 * 
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

// asmlinkage long sys_write_kv(int k, int v); 449
SYSCALL_DEFINE2(write_kv, int, k, int, v)
{
    return 0;
}


// asmlinkage long sys_read_kv(int k); 450
SYSCALL_DEFINE1(read_kv, int, k)
{
    return 0;
}