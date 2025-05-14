
struct task_info {
    struct task_struct *kaddr; // 任务结构体指针(内核虚地址)
    pid_t pid;
};

