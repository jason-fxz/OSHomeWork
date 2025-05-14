
struct my_task_struct_view {
    int page_offset; // 任务结构体在页内的偏移
    pid_t pid; // 进程ID
};

struct task_info {
    struct task_struct *kaddr; // 任务结构体指针(内核虚地址)
    pid_t pid;
};

