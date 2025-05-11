from fuse import FUSE, Operations, FuseOSError
import errno

class HelloFS(Operations):
    def getattr(self, path, fh=None):
        if path == '/':
            return dict(st_mode=(0o755 | 0o040000), st_nlink=2)
        elif path == '/hello.txt':
            return dict(st_mode=(0o666 | 0o100000), st_size=len(self.hello), st_nlink=1)
        else:
            raise FuseOSError(errno.ENOENT)

    def readdir(self, path, fh):
        return ['.', '..', 'hello.txt']

    def read(self, path, size, offset, fh):
        if path == '/hello.txt':
            return self.hello[offset:offset+size]
        else:
            return b''

    def open(self, path, flags):
        if path != '/hello.txt':
            raise FuseOSError(errno.ENOENT)
        return 0

    def __init__(self):
        self.hello = "Hello, world!\n".encode()

    def write(self, path, data, offset, fh):
        if path == '/hello.txt':
            self.hello = self.hello[:offset] + data + self.hello[offset + len(data):]
            return len(data)
        else:
            raise FuseOSError(errno.ENOENT)
    
    def truncate(self, path, length,fh=None):
        if path == '/hello.txt':
            if length < len(self.hello):
                self.hello = self.hello[:length]
            elif length > len(self.hello):
                self.hello += b'\0' * (length - len(self.hello))
            return length
        else:
            raise FuseOSError(errno.ENOENT)
    

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print("Usage: python hello_fuse.py <mountpoint>")
        exit(1)
    mountpoint = sys.argv[1]
    FUSE(HelloFS(), mountpoint, nothreads=True, foreground=True, allow_other=True, ro=False)

