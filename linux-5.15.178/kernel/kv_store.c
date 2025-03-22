/**
 * KV_STORE syscalls
 * 
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>

// define the kv_node struct
struct kv_node {
    int key;
    int value;
    struct hlist_node node;
};

// asmlinkage long sys_write_kv(int k, int v); 449
SYSCALL_DEFINE2(write_kv, int, k, int, v)
{
    struct kv_node *entry, *old_entry = NULL;
    struct hlist_node *tmp;
    unsigned int hash = k & 1023; // k % 1024

    // get current task
    struct task_struct *task = current;

    spin_lock(&task->kv.locks[hash]);

    hlist_for_each_entry(entry, &task->kv.kv_store[hash], node) {
        if (entry->key == k) {
            old_entry = entry;
            break;
        }
    }

    if (old_entry != NULL) {
        // update the value
        old_entry->value = v;
    } else {
        // create a new entry
        entry = kmalloc(sizeof(struct kv_node), GFP_KERNEL);
        if (!entry) {
            spin_unlock(&task->kv.locks[hash]);
            return -1; // memory allocation failed
        }

        entry->key = k;
        entry->value = v;
        hlist_add_head(&entry->node, &task->kv.kv_store[hash]);
    }
    spin_unlock(&task->kv.locks[hash]);
    return sizeof(int);
}


// asmlinkage long sys_read_kv(int k); 450
SYSCALL_DEFINE1(read_kv, int, k)
{
    struct kv_node *entry;
    int ret = -1;
    unsigned int hash = k & 1023; // k % 1024

    // get current task
    struct task_struct *task = current;

    spin_lock(&task->kv.locks[hash]);

    hlist_for_each_entry(entry, &task->kv.kv_store[hash], node) {
        if (entry->key == k) {
            ret = entry->value;
            break;
        }
    }

    spin_unlock(&task->kv.locks[hash]);
    return ret;
}