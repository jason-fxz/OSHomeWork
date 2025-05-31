#include <stdio.h>
#include <stdlib.h>
#include <sys/xattr.h>
#include <asm/unistd.h>
#include <unistd.h>

/*
C standard library functions for extended attributes:
getxattr
setxattr
removexattr
listxattr
*/

/*
from /usr/include/asm/unistd_64.h

#define __NR_setxattr 188
#define __NR_lsetxattr 189
#define __NR_fsetxattr 190
#define __NR_getxattr 191
#define __NR_lgetxattr 192
#define __NR_fgetxattr 193
#define __NR_listxattr 194
#define __NR_llistxattr 195
#define __NR_flistxattr 196
#define __NR_removexattr 197
#define __NR_lremovexattr 198
#define __NR_fremovexattr 199

*/


int get_xattr(const char *path, const char *name, char *value, size_t size, int flags) {
   return syscall(__NR_getxattr, path, name, value, size, flags);
}

int set_xattr(const char *path, const char *name, const char *value, size_t size, int flags) {
   return syscall(__NR_setxattr, path, name, value, size, flags);
}

int remove_xattr(const char *path, const char *name) {
   return syscall(__NR_removexattr, path, name);
}

ssize_t list_xattr(const char *path, char *list, size_t size) {
   return syscall(__NR_listxattr, path, list, size);
}

