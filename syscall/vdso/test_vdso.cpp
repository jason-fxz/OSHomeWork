#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <dlfcn.h>
#include <sys/wait.h>

extern "C" {

struct task_info {
    void *task_struct_ptr;
    pid_t pid;
    // other key fields...
};

int get_task_struct_info(struct task_info *info);


static void *func_sym = NULL;

int get_task_struct_info(struct task_info *info) {
    if (func_sym == NULL) {
        void *handle = dlopen("linux-vdso.so.1", RTLD_LAZY);
        if (!handle) {
            std::cerr << "vdso not found" << std::endl;
            return -1;
        }
        func_sym = dlsym(handle, "__vdso_get_task_struct_info");
        if (!func_sym) {
            std::cerr << "symbol not found" << std::endl;
            dlclose(handle);
            return -1;
        }
    }
    int (*get_task_struct_info_func)(struct task_info *out) = (int (*)(struct task_info *))func_sym;
    int ret = get_task_struct_info_func(info);
    return ret;
}

}

int main() {
    task_info info;
    if (get_task_struct_info(&info) != 0) {
        std::cerr << "Failed to get task struct info" << std::endl;
        return 1;
    }
    pid_t pid_vdso = info.pid;
    std::ifstream stat_file("/proc/self/stat");
    if (!stat_file.is_open()) {
        std::cerr << "Failed to open /proc/self/stat" << std::endl;
        return 1;
    }
    pid_t pid_proc;
    stat_file >> pid_proc;
    stat_file.close();
    if (pid_proc != pid_vdso) {
        std::cerr << "PID mismatch: pid from /proc/self/stat is " << pid_proc
                  << ", pid from vDSO is " << pid_vdso << std::endl;
    } else {
        std::cout << "PID matches: " << pid_proc << std::endl;
    }
    const int num_forks = 5;
    for (int i = 0; i < num_forks; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Failed to fork" << std::endl;
            return 1;
        } else if (pid == 0) {
            task_info child_info;
            if (get_task_struct_info(&child_info) != 0) {
                std::cerr << "Child failed to get task struct info" << std::endl;
                exit(1);
            }
            pid_t child_pid_vdso = child_info.pid;
            std::ifstream child_stat_file("/proc/self/stat");
            if (!child_stat_file.is_open()) {
                std::cerr << "Child failed to open /proc/self/stat" << std::endl;
                exit(1);
            }
            pid_t child_pid_proc;
            child_stat_file >> child_pid_proc;
            child_stat_file.close();
            if (child_pid_proc != child_pid_vdso) {
                std::cerr << "Child PID mismatch: pid from /proc/self/stat is " << child_pid_proc
                          << ", pid from vDSO is " << child_pid_vdso << std::endl;
            } else {
                std::cout << "Child PID matches: " << child_pid_proc << std::endl;
            }
            exit(0);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}
