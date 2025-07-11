# OS 2024 HomeWork

See tutorial at [https://github.com/peterzheng98/os-2024-tutorial](https://github.com/peterzheng98/os-2024-tutorial).

## TODO

### 系统启动

- [x] 基础：Read ACPI Table
  - [x] learn: [ACPI Table](https://blog.csdn.net/u011280717/article/details/124959776)
  - [impl](./HelloWorldPkg/HelloWorld.c)
- [x] 实践：Hack ACPI Table
  - [impl](./HelloWorldPkg/HelloWorld.c)
- [x] 设计：UEFI 运行时服务
  - [x] learn: [DXE Runtime Driver](https://tianocore-docs.github.io/edk2-ModuleWriteGuide/draft/8_dxe_drivers_non-uefi_drivers/88_dxe_runtime_driver.html)
  - [impl](./HardwareInfoAppPkg)

### 系统调用

[syscall](./syscall/)

- [x] 基础：用户定义的系统调用
  - [x] learn: [Adding a New System Call](https://www.kernel.org/doc/html/v5.15/process/adding-syscalls.html)
  - [x] learn: Adding a New field to the `task_struct` [1](https://stackoverflow.com/questions/8044652/adding-entry-to-task-struct-and-initializing-to-default-value) [2](https://www.linuxquestions.org/questions/programming-9/adding-a-new-field-to-task_struct-310638/)
  - [impl](./syscall/kv_syscall/)
- [x] 实践：vDSO
  - [x] learn [vDSO](https://zhuanlan.zhihu.com/p/436454953)
  - [impl](./syscall/vdso/)
- [ ] 设计：无需中断的系统调用

### 内存管理

- [x] 基础：页表和文件页
  - [impl](./memctrl/mmap/)
- [x] 实践：内存文件系统
  - [impl](./memctrl/ramfs/)
- [ ] 设计：内存压力导向的内存管理

### 文件系统

- [x] 基础：inode 和扩展属性管理
  - [impl](./filesystem/inode/)
- [x] 实践：FUSE
  - [impl](./filesystem/fuse/)
- [ ] 设计：用户空间下的内存磁盘

### 网络与外部设备

- [x] 基础：tcpdump 和 socket 管理
  - [x] learn: [数据包结构](https://zhuanlan.zhihu.com/p/532166995)
  - [x] tcpdump [impl](./network/tcpdump/)
  - [x] socket [impl](./network/socket/)
- [x] 实践：NCCL
  - [impl](./network/nccl/)
- [ ] 设计：DPDK
