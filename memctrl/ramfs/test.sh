#!/bin/sh

mkdir -p /tmp/ramfs_sync
mkdir -p /mnt/ramfs

mount -t ramfs -o sync_dir=/tmp/ramfs_sync none /mnt/ramfs

echo "test data" > /mnt/ramfs/test.txt
mkdir -p /mnt/ramfs/233
echo "test2 data" > /mnt/ramfs/233/test2.txt


ls -al /mnt/ramfs/

fsync /mnt/ramfs/test.txt
fsync /mnt/ramfs/233/test2.txt

ls -al /tmp/ramfs_sync/
cat /tmp/ramfs_sync/test.txt

touch /mnt/ramfs/testfile_single.dat
touch /mnt/ramfs/multitest.dat
touch /mnt/ramfs/crashtest.dat