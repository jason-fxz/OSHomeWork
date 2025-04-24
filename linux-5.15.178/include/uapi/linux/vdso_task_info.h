#ifndef _UAPI_LINUX_VDSO_TASK_INFO_H
#define _UAPI_LINUX_VDSO_TASK_INFO_H

#include <linux/types.h>

struct task_info {
    pid_t pid;                   /* 进程 ID */
    void *task_struct_ptr;       /* task_struct 指针 */
};

int get_task_struct_info(struct task_info *info);

#endif /* _UAPI_LINUX_VDSO_TASK_INFO_H */