"""
目标：使用 FUSE 实现 GPT 服务。声明一个 GPTfs，其中的目录为每一个对话 session，对话 session 文件夹
下的 input 为本轮用户输入的 prompt，output 文件为 GPT 输出的结果。每一轮对话都是单轮对话，无需考虑记录
上下文。具体而言，你需要实现：
实现一个名为 GPTfs 的 FUSE 文件系统。
文件系统的根目录下，每次创建一个新目录即表示开启一个新的对话会话（session）。
在每个对话 session 目录下，包含以下文件：
1. input：用于存放本轮用户输入的 prompt。
2. output：用于存放 GPT 生成的回复结果。
3. error：用于存放网络连接错误等其他错误内容。
对 input 文件的写入操作应触发 GPT 处理，生成回复内容并写入对应的 output 文件。
每一轮对话都是单轮对话，无需记录上下文。

"""

from fuse import FUSE, Operations, FuseOSError
import errno
import os
import threading
import time
import stat
import requests


def call_ollama(prompt : str, model="qwen3:8b") -> str:
    try:
        response = requests.post(
            "http://localhost:11434/api/generate",
            json={
                "model": model,
                "prompt": prompt,
                "stream": False
            },
            timeout=30
        )
        response.raise_for_status()
        return response.json()["response"]
    except Exception as e:
        raise RuntimeError(f"Ollama error: {e}")
    
class GPTfs(Operations):
    def __init__(self):
        # self.lock = threading.Lock()
        self.sessions = {}  # 每个会话的内容状态 'key': {input: bytes, output: bytes, error: bytes}


    def readdir(self, path, fh):
        print(f"DEBUG readdir path={path}")
        parts = path.strip("/").split("/")
        if path == '/': # root dir
            return ['.', '..'] + list(self.sessions.keys())
        elif len(parts) == 1: # session dir
            session = parts[0]
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            return ['.', '..', 'input', 'output', 'error']
        else:
            raise FuseOSError(errno.ENOTDIR)


    def getattr(self, path, fh=None):
        print(f"DEBUG getattr path={path}")
        now = int(time.time())
        parts = path.strip("/").split("/")
        if path == '/': # root dir
            mode = stat.S_IFDIR | 0o755
            return dict(st_mode=mode, st_nlink=2, st_ctime=now, st_mtime=now, st_atime=now)
        elif len(parts) == 1: # session dir
            # check if session exists
            if parts[0] not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            return dict(st_mode=(stat.S_IFDIR | 0o755), st_nlink=2, st_ctime=now, st_mtime=now, st_atime=now)
        elif len(parts) == 2:
            session, filename = parts
            # check if session exists
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            # input rw; output r; error r
            if filename in ['input']:
                content = self.sessions.get(session).get(filename)
                return dict(st_mode=(stat.S_IFREG | 0o644), st_nlink=1,
                            st_size=len(content), st_ctime=now, st_mtime=now, st_atime=now)
            elif filename in ['output', 'error']:
                content = self.sessions.get(session).get(filename)
                return dict(st_mode=(stat.S_IFREG | 0o444), st_nlink=1,
                            st_size=len(content), st_ctime=now, st_mtime=now, st_atime=now)
            else:
                raise FuseOSError(errno.ENOENT)
        else:
            raise FuseOSError(errno.ENOENT)

    def mkdir(self, path, mode):
        print(f"DEBUG mkdir path={path} mode={mode}")
        parts = path.strip("/").split("/")
        if len(parts) == 1:
            session = parts[0]
            # with self.lock:
            if session not in self.sessions:
                self.sessions[session] = {
                    "input": b"",
                    "output": b"",
                    "error": b"",
                }
            else:
                raise FuseOSError(errno.EEXIST)
        else:
            raise FuseOSError(errno.ENOENT)

    def read(self, path, size, offset, fh):
        print(f"DEBUG read path={path} size={size} offset={offset}")
        parts = path.strip("/").split("/")
        if len(parts) == 2:
            session, filename = parts
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            if filename not in ['input', 'output', 'error']:
                raise FuseOSError(errno.ENOENT)
            # with self.lock:
            content = self.sessions.get(session).get(filename)
            return content[offset:offset+size]
        raise FuseOSError(errno.ENOENT)

    def write(self, path, data, offset, fh):
        print(f"DEBUG write path={path} data={data} offset={offset}")
        parts = path.strip("/").split("/")
        if len(parts) == 1:
            raise FuseOSError(errno.ENOENT)
        elif len(parts) == 2:
            session, filename = parts
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            # 只允许写入 input 文件
            if filename != 'input':
                raise FuseOSError(errno.ENOENT)
            
            # with self.lock:
            self.sessions[session]["input"] = self.sessions[session]["input"][:offset] + data + self.sessions[session]["input"][offset + len(data):]
            try:
                output = call_ollama(self.sessions[session]["input"].decode()).encode()
                self.sessions[session]["output"] = output
                self.sessions[session]["error"] = ""
            except Exception as e:
                self.sessions[session]["output"] = ""
                self.sessions[session]["error"] = str(e).encode()
            return len(data)
        else:
            raise FuseOSError(errno.ENOENT)

    def open(self, path, flags):
        print(f"DEBUG open path={path} flags={flags}")
        parts = path.strip("/").split("/")
        if len(parts) == 2:
            session, filename = parts
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            if filename not in ['input', 'output', 'error']:
                raise FuseOSError(errno.ENOENT)
            return 0
        raise FuseOSError(errno.ENOENT)
        
    def truncate(self, path, length, fh=None):
        parts = path.strip("/").split("/")
        if len(parts) == 2:
            session, filename = parts
            if session not in self.sessions:
                raise FuseOSError(errno.ENOENT)
            if filename != 'input':
                raise FuseOSError(errno.ENOENT)
            # with self.lock:
            if length < 0:
                raise FuseOSError(errno.EINVAL)
            content = self.sessions[session][filename]
            if length < len(content):
                self.sessions[session][filename] = content[:length]
            elif length > len(content):
                self.sessions[session][filename] = content + b'\0' * (length - len(content))

            return length
        raise FuseOSError(errno.ENOENT)



if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print("Usage: python GPT_FUSE.py <mountpoint>")
        exit(1)
    mountpoint = sys.argv[1]
    FUSE(GPTfs(), mountpoint, nothreads=True, foreground=True)

