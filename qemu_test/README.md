# Qemu Test

## 使用 BusyBox 创建一文件系统

```shell
# 创建目录
mkdir -p rootfs
# 创建磁盘镜像(1GB)
dd if=/dev/zero of=rootfs.img bs=1M count=1024
# 格式化磁盘
mkfs.ext4 rootfs.img
# 挂载文件系统
sudo mount rootfs.img rootfs
# BusyBox 
sudo mkdir -p rootfs/{bin,sbin,etc,proc,sys,dev,usr/{bin,sbin}}
sudo cp /bin/busybox rootfs/bin
cd rootfs
sudo ln -s bin/busybox init
for prog in $(./bin/busybox --list); do
    ln -s bin/busybox bin/$prog;
done
# 取消挂载
sudo umount rootfs
```
