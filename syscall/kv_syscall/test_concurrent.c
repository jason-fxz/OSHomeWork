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
#define NUM_KEYS 100

pthread_barrier_t barrier;

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
        key = rand() % NUM_KEYS + 1024 * (rand() & 3);

        if (rand() & 1)
        { // 50%概率写操作
            val = key;
            ret = write_kv(key, val);
            if (ret == -1)
            {
                printf("Thread %d: write_kv failed, key=%d, val=%d, ret=%d\n",
                       thread_id, key, val, ret);
                exit(1);
            }
        }
        else
        { // 50%概率读操作
            val = read_kv(key);
            if (val != -1) {
                if (val != key) {
                    printf("Thread %d: read_kv failed, key=%d, expected=%d, got=%d\n",
                        thread_id, key, key, val);
                    exit(1);
                }
            } else {
                // printf("Thread %d: read_kv -1, key=%d\n",
                //     thread_id, key);
                // exit(1);
            }
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



    pthread_barrier_destroy(&barrier);
    printf("Concurrent test PASSED! All expected values match.\n");
    return 0;
}