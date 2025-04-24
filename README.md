# OS 2024 HomeWork

See tutorial at [https://github.com/peterzheng98/os-2024-tutorial](https://github.com/peterzheng98/os-2024-tutorial).

## TODO

### 系统启动

- [x] 基础：Read ACPI Table
  - [x] learn: [ACPI Table](https://blog.csdn.net/u011280717/article/details/124959776)
  - [./HelloWorldPkg/HelloWorld.c](./HelloWorldPkg/HelloWorld.c)
- [x] 实践：Hack ACPI Table
  - [./HelloWorldPkg/HelloWorld.c](./HelloWorldPkg/HelloWorld.c)
- [ ] 设计：UEFI 运行时服务

### 系统调用

[syscall](./syscall/)

- [x] 基础：用户定义的系统调用
  - [x] learn: [Adding a New System Call](https://www.kernel.org/doc/html/v5.15/process/adding-syscalls.html)
  - [x] learn: Adding a New field to the `task_struct` [1](https://stackoverflow.com/questions/8044652/adding-entry-to-task-struct-and-initializing-to-default-value) [2](https://www.linuxquestions.org/questions/programming-9/adding-a-new-field-to-task_struct-310638/)
- [x] 实践：vDSO
  - [x] learn [vDSO](https://zhuanlan.zhihu.com/p/436454953)
- [ ] 设计：无需中断的系统调用

### 内存管理

- [ ] 基础：页表和文件页
- [ ] 实践：内存文件系统
- [ ] 设计：内存压力导向的内存管理

### 文件系统

- [ ] 基础：inode 和扩展属性管理
- [ ] 实践：FUSE
- [ ] 设计：用户空间下的内存磁盘

### 网络与外部设备

- [ ] 基础：tcpdump 和 socket 管理
- [ ] 实践：NCCL
- [ ] 设计：DPDK
