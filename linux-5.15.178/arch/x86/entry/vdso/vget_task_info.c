/*
 * vget_task_info.c: vDSO to get task info
 */

#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <asm/vdso.h>
#include <uapi/linux/vdso_task_info.h>


notrace long
__vdso_get_task_info(struct task_info __user *info)
{
    struct task_struct *tsk = current;
    
    if (!info || !access_ok(info, sizeof(*info)))
        return -EFAULT;
    
    if (copy_to_user(&info->pid, &tsk->pid, sizeof(tsk->pid)))
        return -EFAULT;

    info->task_struct_ptr = (void *)tsk;


    return 0;
}

long get_task_struct_info(struct task_info *info)
    __attribute__((weak, alias("__vdso_get_task_info")));