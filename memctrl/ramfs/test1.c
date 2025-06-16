// Single-file persistence and correctness Test
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define RAMFS_FILE "/mnt/ramfs/testfile_single.dat"
#define FILE_SIZE_MB 64
#define TIMER_INTERVAL_SECONDS 5

void random_fill(char *buf, int size) {
    for (int i = 0; i < size; i++)
        buf[i] = rand() % 256;
}

int main() {
    int fd = open(RAMFS_FILE, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("Failed to open RAMFS file");
        exit(1);
    }

    int total_size = FILE_SIZE_MB * 1024 * 1024;
    int chunk = 1 * 1024 * 1024; // 1MB per write
    char *buf = malloc(chunk);
    time_t last_flush = time(NULL);

    printf("Starting single file persistence test...\n");

    for (int written = 0; written < total_size; written += chunk) {
        random_fill(buf, chunk);

        if(write(fd, buf, chunk) != chunk) {
            perror("Write error");
            exit(1);
        }

        printf("Written %d MB\n", (written + chunk) / (1024 * 1024));

        // Immediate flush after first 1MB written
        if (written == 0) {
            fsync(fd);
            printf("Immediate flush triggered.\n");
        }

        // Periodic flush every TIMER_INTERVAL_SECONDS
        if (time(NULL) - last_flush >= TIMER_INTERVAL_SECONDS) {
            fsync(fd);
            printf("Periodic flush triggered at %d MB.\n", (written + chunk) / (1024 * 1024));
            last_flush = time(NULL);
        }
    }

    // Final flush test
    fsync(fd);
    printf("Final flush successful.\n");

    free(buf);
    close(fd);

    printf("Single-file persistence test completed. Verify correctness manually.\n");
    return 0;
}

