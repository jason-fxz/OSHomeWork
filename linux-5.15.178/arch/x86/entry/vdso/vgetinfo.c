#include <linux/kernel.h>
#include <linux/user_taskinfo.h>
// #include <asm/processor.h>
#include <linux/types.h>
#include <linux/sched.h>
// #define CS_BASES 2
#include <vdso/datapage.h>
#include <asm/vvar.h>


// vvar 区域大小，通常为 4 * PAGE_SIZE
#define VVAR_SIZE   (4 * PAGE_SIZE)
#define VTASK_SIZE  ALIGN(sizeof(struct task_struct), PAGE_SIZE)

extern char vvar_page;

static inline void *get_task_addr(void)
{
    // vtask 区域在 vvar 区域上方
    return (void *)(&vvar_page - VTASK_SIZE);
}

int __vdso_get_task_struct_info(struct task_info *info)
{
    struct task_struct *task;
    if (!info)
        return -1;
    
    // 直接从映射的内存中读取task_struct
    task = (struct task_struct *)get_task_addr();
        
    info->kaddr = task;
    info->pid = task->pid;
    
    return 0;
}

// 这是用户空间调用的包装函数
int get_task_struct_info(struct task_info *info)
{
    return __vdso_get_task_struct_info(info);
}
