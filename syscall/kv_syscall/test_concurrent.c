#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "kv_syscalls.h"

#define NUM_THREADS 100
#define ITERATIONS 10000
#define NUM_KEYS 10

pthread_barrier_t barrier;
pthread_mutex_t final_values_mutex[NUM_KEYS];
int final_values[NUM_KEYS];

void *thread_func(void *arg)
{
    // printf("Thread %d started\n", *(int *)arg);
    int thread_id = *(int *)arg;
    int i, key, val, ret;

    // 初始化随机数种子
    srand(time(NULL) + thread_id);

    // 等待所有线程就绪
    pthread_barrier_wait(&barrier);
    // printf("Thread %d running\n", thread_id);

    // 执行随机读写操作
    for (i = 0; i < ITERATIONS; i++)
    {
        key = rand() % NUM_KEYS;

        if (rand() & 1)
        { // 50%概率写操作
            val = thread_id * ITERATIONS + i;
            ret = write_kv(key, val);
            if (ret == -1)
            {
                printf("Thread %d: write_kv failed, key=%d, val=%d, ret=%d\n",
                       thread_id, key, val, ret);
            }
            else
            {
                pthread_mutex_lock(&final_values_mutex[key]);
                // printf("Thread %d: write_kv success, key=%d, val=%d\n", thread_id, key, val);
                final_values[key] = val;
                pthread_mutex_unlock(&final_values_mutex[key]);
            }
        }
        else
        { // 50%概率读操作
            val = read_kv(key);
            // printf("Thread %d: read_kv, key=%d, get val=%d\n", thread_id, key, val);
        }
    }

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int i, val;

    printf("Starting concurrent test with %d threads...\n", NUM_THREADS);
    // 初始化
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    for (i = 0; i < NUM_KEYS; i++)
        pthread_mutex_init(&final_values_mutex[i], NULL);
    memset(final_values, -1, sizeof(final_values));

    // 创建线程
    for (i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    printf("Waiting for threads to finish...\n");

    // 等待所有线程完成
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // 验证最终的值
    for (i = 0; i < NUM_KEYS; i++)
    {
        if (final_values[i] != -1)
        {
            val = read_kv(i);
            if (val != final_values[i])
            {
                printf("Key %d: expected %d, got %d\n", i, final_values[i], val);
                assert(0);
            }
        }
    }

    for (i = 0; i < NUM_KEYS; i++)
        pthread_mutex_destroy(&final_values_mutex[i]);

    pthread_barrier_destroy(&barrier);
    printf("Concurrent test PASSED! All expected values match.\n");
    return 0;
}