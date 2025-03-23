#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include "kv_syscalls.h"

int main() {
    int ret, val;
    
    printf("Testing basic write/read functionality...\n");
    
    // Test 1: Write and read back a value
    ret = write_kv(114514, 1234);
    assert(ret == sizeof(int));
    
    val = read_kv(114514);
    assert(val == 1234);
    printf("Test 1 passed: write/read single value\n");
    
    // Test 2: Overwrite a value
    ret = write_kv(114514, 5678);
    assert(ret == sizeof(int));
    
    val = read_kv(114514);
    assert(val == 5678);
    printf("Test 2 passed: overwrite value\n");
    
    // Test 3: Read non-existent key
    val = read_kv(999);
    assert(val == -1);
    printf("Test 3 passed: read non-existent key\n");
    
    // Test 4: Multiple keys
    for (int i = 0; i < 100; i++) {
        ret = write_kv(i, i * 10);
        assert(ret == sizeof(int));
    }
    
    for (int i = 0; i < 100; i++) {
        val = read_kv(i);
        assert(val == i * 10);
    }
    printf("Test 4 passed: multiple keys\n");
    
    printf("All basic tests PASSED!\n");
    return 0;
}