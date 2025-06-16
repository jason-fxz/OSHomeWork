// Crash Recovery & Exception Handling Test
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TESTFILE "/mnt/ramfs/crashtest.dat"

int main() {
    int fd = open(TESTFILE, O_CREAT | O_WRONLY, 0666);
    if(fd < 0) { perror("Cannot open test file"); exit(1); }

    char data[1024 * 1024] = {0}; // 1MB zeroed data

    printf("Writing data to trigger flush...\n");
    write(fd, data, sizeof(data));

    printf("Calling flush before triggering kernel panic...\n");
    fsync(fd); // Ensure data is flushed to disk

    // Trigger panic immediately after flush
    printf("Triggering kernel panic for crash simulation...\n");
    int kfd = open("/proc/sysrq-trigger", O_WRONLY);
    if (kfd < 0) { perror("Cannot open sysrq-trigger (root privilege needed)"); exit(1); }
    write(kfd, "c", 1);
    close(kfd);

    close(fd);
    return 0; // Unreachable but syntactically correct
}
// After system reboot, users must manually check file and integrity:
// 1. Mount RAMfs again.
// 2. Verify no orphan inode, and integrity constraints are strictly maintained.
// This manual check is necessary as system is rebooted.

