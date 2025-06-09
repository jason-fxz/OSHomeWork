/*
CONFIG_KASAN=y
CONFIG_KASAN_INLINE=y
CONFIG_LOCKDEP=y
CONFIG_DEBUG_SPINLOCK=y
CONFIG_DEBUG_MUTEXES=y
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/syscalls.h>


#define NUM_THREADS 100
#define NUM_ITERATIONS 1000
#define MAX_KEY 2048

// 声明系统调用函数
extern asmlinkage long kv_store_write(int k, int v);
extern asmlinkage long kv_store_read(int k);

// 包装函数
static inline int write_kv(int k, int v)
{
    return kv_store_write(k, v);
}

static inline int read_kv(int k)
{
    return kv_store_read(k);
}

static struct task_struct *threads[NUM_THREADS];

static int kv_test_thread(void *arg) {
    int i;
    for (i = 0; i < NUM_ITERATIONS && !kthread_should_stop(); i++) {
        int k, v, operation;
        get_random_bytes(&k, sizeof(k));
        k = abs(k) % MAX_KEY;
        get_random_bytes(&v, sizeof(v));
        operation = k % 2; 

        if (operation) {
            
            if (write_kv(k, v) == -1) {
                pr_err("Error writing key %d\n", k);
            }
        } else {
            
            int read_value = read_kv(k);
            // if (read_value == -1) {
            //     pr_err("Error reading key %d\n", k);
            // }
        }

        
        schedule();
    }
    return 0;
}

static int __init kv_test_init(void) {
    int i;
    pr_info("KV Test Module with KASAN and LOCKDEP starting.\n");

    for (i = 0; i < NUM_THREADS; i++) {
        threads[i] = kthread_run(kv_test_thread, NULL, "kv_test_thread_%d", i);
        if (IS_ERR(threads[i])) {
            pr_err("Failed to create thread %d\n", i);
            threads[i] = NULL;
        }
    }
    return 0;
}

static void __exit kv_test_exit(void) {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        if (threads[i]) {
            kthread_stop(threads[i]);
        }
    }
    pr_info("KV Test Module exiting.\n");
}

module_init(kv_test_init);
module_exit(kv_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module to test KV store with KASAN and LOCKDEP");
MODULE_AUTHOR("Your Name");
