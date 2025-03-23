#ifndef _KV_SYSCALLS_H
#define _KV_SYSCALLS_H

#include <unistd.h>
#include <sys/syscall.h>

#ifndef __NR_write_kv
#define __NR_write_kv 449
#endif

#ifndef __NR_read_kv
#define __NR_read_kv 450
#endif

/**
 * write a key-value pair
 * @param k key
 * @param v value
 * @return success return the number of bytes written, fail return -1
 */
static inline int write_kv(int k, int v)
{
    return syscall(__NR_write_kv, k, v);
}

/**
 * read a value by key
 * @param k key
 * @return success return the value, fail return -1
 */
static inline int read_kv(int k)
{
    return syscall(__NR_read_kv, k);
}

#endif // _KV_SYSCALLS_H