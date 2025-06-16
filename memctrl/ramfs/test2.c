// Multi-threaded Sequential Consistency & Robustness Test
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define THREAD_PRODUCERS 10
#define THREAD_CONSUMERS 3
#define ITERATIONS 1000
#define FILEPATH "/mnt/ramfs/multitest.dat"
#define ENTRY_SIZE 128

pthread_mutex_t lock;

void *producer_thread(void *arg) {
    int fd = open(FILEPATH, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if(fd < 0) { perror("Producer open failed"); pthread_exit(NULL); }

    char entry[ENTRY_SIZE];
    for(int i = 0; i < ITERATIONS; i++) {
        snprintf(entry, ENTRY_SIZE, "[TID-%ld] timestamp: %ld iteration: %d\n", pthread_self(), time(NULL), i);
        pthread_mutex_lock(&lock);
        if(write(fd, entry, strlen(entry)) < 0) perror("Producer write failed");
        pthread_mutex_unlock(&lock);
        usleep(rand() % 1000); // simulate varying loads
    }
    close(fd);
    pthread_exit(NULL);
}

void *consumer_thread(void *arg) {
    FILE *fp = fopen(FILEPATH, "r+");
    if(fp == NULL) { perror("Consumer open failed"); pthread_exit(NULL); }
    for(int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&lock);
        fsync(fileno(fp)); // ensure file is up-to-date
        pthread_mutex_unlock(&lock);
        usleep((rand() % 2000) + 500);
    }
    fclose(fp);
    pthread_exit(NULL);
}

int main() {
    pthread_t producers[THREAD_PRODUCERS], consumers[THREAD_CONSUMERS];
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);

    printf("Starting multi-threaded sequential consistency test...\n");

    for(int i = 0; i < THREAD_PRODUCERS; i++)
        pthread_create(&producers[i], NULL, producer_thread, NULL);
    for(int i = 0; i < THREAD_CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer_thread, NULL);

    for(int i = 0; i < THREAD_PRODUCERS; i++)
        pthread_join(producers[i], NULL);
    for(int i = 0; i < THREAD_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    pthread_mutex_destroy(&lock);
    printf("Multi-threaded test completed. Verify sequential consistency manually (check Timestamps).\n");
    return 0;
}

