#include "impl.c"
#include <assert.h>
#include <bits/mman-linux.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern void *mmap_remap(void *addr, size_t size);
extern int file_mmap_write(const char *filename, size_t offset, char *content);

#define PAGE_SIZE 4096
#define LARGE_SIZE (1024 * 1024) // 1MB

int is_root() {
  return (geteuid() == 0);
}

uint64_t get_physical_address(void *virtual_address) {
  int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);

  uint64_t offset = ((uintptr_t)virtual_address / PAGE_SIZE) * sizeof(uint64_t);
  uint64_t entry;

  if (pread(pagemap_fd, &entry, sizeof(entry), offset) != sizeof(entry)) {
    close(pagemap_fd);
    return 0;
  }

  close(pagemap_fd);

  if (!(entry & (1ULL << 63))) {
    printf("Page not present in memory\n");
    return 0;
  }

  return (entry & ((1ULL << 54) - 1)) * PAGE_SIZE;
}

// Helper function to create test files
void create_test_file(const char *filename, size_t size) {
  FILE *fp = fopen(filename, "wb");
  if (fp) {
    for (size_t i = 0; i < size; i++) {
      fputc('A' + (i % 26), fp);
    }
    fclose(fp);
  }
}

// Helper function to compare file content using hexdump
int compare_files(const char *file1, const char *file2) {
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "cmp %s %s > /dev/null 2>&1", file1, file2);
  return system(cmd);
}

void test_mmap_remap() {
  printf("\n=== Testing mmap_remap ===\n");

  // Allocate initial memory
  size_t size = PAGE_SIZE * 2;
  void *addr1 = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // Write pattern to memory
  memset(addr1, 0xAA, size);
  // Remap memory
  void *addr2 = mmap_remap(addr1, size);
  assert(addr2 != NULL);

  // Verify addresses are different
  assert(addr1 != addr2);
  assert(get_physical_address(addr1) != get_physical_address(addr2)),
      "Physical addresses are same";

  // Test pattern preservation
  for (size_t i = 0; i < size; i++) {
    assert(((unsigned char *)addr2)[i] == 0xAA);
  }

  // Cleanup
  munmap(addr1, size);
  munmap(addr2, size);
}

void test_file_operations(const char *filename, size_t filesize) {
  printf("\n=== Testing file operations for %s (size: %zu) ===\n", filename,
         filesize);

  // Create test file
  create_test_file(filename, filesize);

  // Test 1: Read-after-write
  char content1[] = "TEST_CONTENT_1";
  assert(file_mmap_write(filename, 0, content1) == 0);

  // Test 2: Cross-page write
  char content2[PAGE_SIZE + 100];
  memset(content2, 'B', sizeof(content2));
  content2[sizeof(content2) - 1] = '\0';
  assert(file_mmap_write(filename, PAGE_SIZE - 50, content2) == 0);

  // Test 3: Append write
  char content3[] = "APPEND_TEST";
  assert(file_mmap_write(filename, filesize, content3) == 0);

  // Create reference file for verification
  char ref_filename[256];
  snprintf(ref_filename, sizeof(ref_filename), "%s.ref", filename);
  printf("Creating reference file: %s\n", ref_filename);
  FILE *fp = fopen(ref_filename, "wb");
  if (fp) {
    for (size_t i = 0; i < filesize; i++) {
      fputc('A' + (i % 26), fp);
    }
    fseek(fp, 0, SEEK_SET);
    fwrite(content1, 1, strlen(content1), fp);
    fseek(fp, PAGE_SIZE - 50, SEEK_SET);
    fwrite(content2, 1, strlen(content2), fp);
    fseek(fp, filesize, SEEK_SET);
    fwrite(content3, 1, strlen(content3), fp);
    fclose(fp);
  }

  // Compare files using hexdump
  if (compare_files(filename, ref_filename) == 0) {
    printf("File comparison successful for %s\n", filename);
  } else {
    printf("File comparison failed for %s\n", filename);
  }
}

int main() {
  // Check for root permissions
  if (!is_root()) {
    fprintf(stderr, "WARNING: This program requires root privileges to access /proc/self/pagemap.\n");
    fprintf(stderr, "Please run with sudo or as root user.\n");
    return 1;
  }

  // Test 1: Page table entry validation
  test_mmap_remap();
  printf("Remapping Passed.\n");
  // Test 2: Memory-file synchronization tests
  const char *empty_file = "empty.txt";
  const char *small_file = "small.txt";
  const char *large_file = "large.txt";

  // Test with different file sizes
  test_file_operations(empty_file, 0);          // Empty file
  test_file_operations(small_file, PAGE_SIZE);  // 4KB file
  test_file_operations(large_file, LARGE_SIZE); // 1MB file

  // Cleanup test files
  unlink(empty_file);
  unlink(small_file);
  unlink(large_file);
  unlink("empty.txt.ref");
  unlink("small.txt.ref");
  unlink("large.txt.ref");

  return 0;
}