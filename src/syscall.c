#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../include/sys/syscall.h"
#include "../include/sys/stat.h"
#include "../include/sys/time.h"
#include "../include/sys/socket.h"
#include "../include/fcntl.h"
#include "../include/unistd.h"
#include "../include/string.h"
#include "../include/time.h"
#include "../include/stdlib.h"
#include "../include/alloca.h"
#include "../include/errno.h"

long syscall(long number, ...) {
    va_list args;
    va_start(args, number);
    long a1 = va_arg(args, long);
    long a2 = va_arg(args, long);
    long a3 = va_arg(args, long);
    va_end(args);

    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(number), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

// sys_write
int write(int fd, const void *buf, size_t count) {
    return (int)syscall(sys_write, fd, (long)buf, count);
}

// sys_read
int read(int fd, void *buf, size_t count) {
    return (int)syscall(sys_read, fd, (long)buf, count);
}

// sys_exit
void _exit(int status) {
    syscall(sys_exit, status, 0, 0);
    while (1) {
        __asm__ volatile("hlt");
    }
}

void exit(int status) {
    _exit(status);
}

// 4. sys_getpid
pid_t getpid(void) {
    return (pid_t)syscall(sys_getpid, 0, 0, 0);
}

// 5. sys_fork
pid_t fork(void) {
    return (pid_t)syscall(sys_fork, 0, 0, 0);
}

// sys_execve
int execve(const char *path, char *const argv[], char *const envp[]) {
    int ret = (int)syscall(sys_execve, (long)path, (long)argv, (long)envp);
    if (ret < 0) errno = ENOENT;
    return ret;
}


// sys_wait4
pid_t wait4(pid_t pid, int *status, int options, void *rusage) {
    (void)rusage;
    pid_t ret = (pid_t)syscall(sys_wait4, pid, (long)status, options);
    if (ret < 0) {
        errno = ECHILD;
    }
    return ret;
}

pid_t waitpid(pid_t pid, int *status, int options) {
    return wait4(pid, status, options, NULL);
}

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}

// sys_getppid
pid_t getppid(void) {
    return (pid_t)syscall(sys_getppid, 0, 0, 0);
}

uid_t getuid(void) { return 0; }
uid_t geteuid(void) { return 0; }
gid_t getgid(void) { return 0; }
gid_t getegid(void) { return 0; }
int getgroups(int size, gid_t list[]) { (void)size; (void)list; return 0; }

// sys_brk
int brk(void *addr) {
    void *new_brk = (void*)syscall(sys_brk, (long)addr, 0, 0);
    if (new_brk == (void*)-1) {
        return -1;
    }
    return 0;
}

// sys_mmap
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)addr;
    (void)prot;
    (void)flags;
    return (void*)syscall(sys_mmap, length, (long)fd, (long)offset);
}


// sys_munmap
int munmap(void *addr, size_t length) {
    return (int)syscall(sys_munmap, (long)addr, length, 0);
}

// sys_pipe
int pipe(int pipefd[2]) {
    return (int)syscall(sys_pipe, (long)pipefd, 0, 0);
}

// sys_dup
int dup(int oldfd) {
    return (int)syscall(sys_dup, oldfd, 0, 0);
}

// sys_dup2
int dup2(int oldfd, int newfd) {
    return (int)syscall(sys_dup2, oldfd, newfd, 0);
}

// sys_kill
int kill(pid_t pid, int sig) {
    return (int)syscall(sys_kill, pid, sig, 0);
}

// sys_ioctl
int ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    void *arg = va_arg(args, void*);
    va_end(args);
    return (int)syscall(sys_ioctl, fd, request, (long)arg);
}

// sys_open
int open(const char *path, int flags, ...) {
    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);
    int ret = (int)syscall(sys_open, (long)path, flags, mode);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

// sys_close
int close(int fd) {
    return (int)syscall(sys_close, fd, 0, 0);
}

// sys_lseek
off_t lseek(int fd, off_t offset, int whence) {
    return (off_t)syscall(sys_lseek, fd, offset, whence);
}

// sys_stat
int stat(const char *path, struct stat *buf) {
    if (buf) {
        unsigned char *p = (unsigned char*)buf;
        for (unsigned int i = 0; i < sizeof(*buf); i++) p[i] = 0;
    }
    int ret = (int)syscall(sys_stat, (long)path, (long)buf, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

// sys_fstat
int fstat(int fd, struct stat *buf) {
    if (buf) {
        unsigned char *p = (unsigned char*)buf;
        for (unsigned int i = 0; i < sizeof(*buf); i++) p[i] = 0;
    }
    return (int)syscall(sys_fstat, fd, (long)buf, 0);
}

// sys_unlink
int unlink(const char *path) {
    int ret = (int)syscall(sys_unlink, (long)path, 0, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

int rmdir(const char *path) {
    int ret = (int)syscall(sys_rmdir, (long)path, 0, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

// sys_mkdir
int mkdir(const char *path, mode_t mode) {
    int ret = (int)syscall(sys_mkdir, (long)path, mode, 0);
    if (ret < 0) {
        errno = EEXIST;
    }
    return ret;
}

// sys_chdir
int chdir(const char *path) {
    int ret = (int)syscall(sys_chdir, (long)path, 0, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

// sys_getcwd
char *getcwd(char *buf, size_t size) {
    int ret = (int)syscall(sys_getcwd, (long)buf, size, 0);
    if (ret < 0) {
        errno = ENOENT;
        return NULL;
    }
    return buf;
}

// sys_ftruncate
int ftruncate(int fd, off_t length) {
    return (int)syscall(sys_ftruncate, fd, length, 0);
}

int truncate(const char *path, off_t length) {
    int ret = (int)syscall(sys_truncate, (long)path, length, 0);
    if (ret == 0) return 0;
    // fallback for old kernels: open + ftruncate
    int fd = open(path, O_WRONLY, 0);
    if (fd < 0) return -1;
    ret = ftruncate(fd, length);
    close(fd);
    return ret;
}

pid_t vfork(void) {
    return fork();
}

int isatty(int fd) {
    return (fd >= 0 && fd <= 2);
}

int access(const char *pathname, int mode) {
    int ret = (int)syscall(sys_access, (long)pathname, mode, 0);
    if (ret == 0) return 0;
    // fallback for old kernels: stat probe
    if (ret < 0) {
        struct stat st;
        if (stat(pathname, &st) == 0) return 0;
    }
    return -1;
}

int lstat(const char *path, struct stat *buf) {
    if (buf) {
        unsigned char *p = (unsigned char*)buf;
        for (unsigned int i = 0; i < sizeof(*buf); i++) p[i] = 0;
    }
    int ret = (int)syscall(sys_lstat, (long)path, (long)buf, 0);
    if (ret == 0) return 0;
    return stat(path, buf);
}

int rename(const char *oldpath, const char *newpath) {
    int ret = (int)syscall(sys_rename, (long)oldpath, (long)newpath, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

mode_t umask(mode_t mask) {
    (void)mask;
    return 022;
}

int chmod(const char *path, mode_t mode) {
    (void)path;
    (void)mode;
    return 0;
}

int fchmod(int fd, mode_t mode) {
    (void)fd;
    (void)mode;
    return 0;
}

int mknod(const char *pathname, mode_t mode, dev_t dev) {
    (void)pathname;
    (void)mode;
    (void)dev;
    return 0;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    (void)pathname;
    (void)owner;
    (void)group;
    return 0;
}

int fchown(int fd, uid_t owner, gid_t group) {
    (void)fd;
    (void)owner;
    (void)group;
    return 0;
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
    (void)pathname;
    (void)owner;
    (void)group;
    return 0;
}

int utimes(const char *filename, const struct timeval times[2]) {
    struct timespec ts[2];
    const struct timespec *tp = NULL;
    if (times) {
        for (int i = 0; i < 2; i++) {
            ts[i].tv_sec = times[i].tv_sec;
            ts[i].tv_nsec = times[i].tv_usec * 1000;
        }
        tp = ts;
    }
    int ret = (int)syscall(sys_utimens, (long)filename, (long)tp, 0);
    if (ret == 0) return 0;
    // old kernel without utimens: no timestamps, pretend success
    return 0;
}

int lutimes(const char *filename, const struct timeval times[2]) {
    struct timespec ts[2];
    const struct timespec *tp = NULL;
    if (times) {
        for (int i = 0; i < 2; i++) {
            ts[i].tv_sec = times[i].tv_sec;
            ts[i].tv_nsec = times[i].tv_usec * 1000;
        }
        tp = ts;
    }
    int ret = (int)syscall(sys_utimens, (long)filename, (long)tp, AT_SYMLINK_NOFOLLOW);
    if (ret == 0) return 0;
    return 0;
}

int futimens(int fd, const struct timespec times[2]) {
    int ret = (int)syscall(sys_futimens, fd, (long)times, 0);
    if (ret == 0) return 0;
    // old kernel without futimens: succeed iff the fd is a real file
    {
        struct stat st;
        extern int fstat(int fd, struct stat *buf);
        if (fstat(fd, &st) == 0) return 0;
    }
    errno = EBADF;
    return -1;
}

int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags) {
    (void)dirfd; // always AT_FDCWD: paths resolve against the cwd
    if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW) {
        errno = EINVAL;
        return -1;
    }
    int ret = (int)syscall(sys_utimens, (long)pathname, (long)times, flags);
    if (ret == 0) return 0;
    // old kernel without utimens: succeed iff the file exists so touch
    // knows whether to create it
    if (!pathname) {
        errno = ENOENT;
        return -1;
    }
    {
        struct stat st;
        int ok;
        if (flags & AT_SYMLINK_NOFOLLOW) ok = lstat(pathname, &st);
        else ok = stat(pathname, &st);
        if (ok == 0) return 0;
    }
    errno = ENOENT;
    return -1;
}

int link(const char *oldpath, const char *newpath) {
    int ret = (int)syscall(sys_link, (long)oldpath, (long)newpath, 0);
    if (ret < 0) errno = ENOENT;
    return ret;
}

int symlink(const char *target, const char *linkpath) {
    int ret = (int)syscall(sys_symlink, (long)target, (long)linkpath, 0);
    if (ret < 0) errno = ENOENT;
    return ret;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    int ret = (int)syscall(sys_readlink, (long)pathname, (long)buf, bufsiz);
    if (ret < 0) errno = ENOENT;
    return ret;
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    return -1;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    (void)sockfd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    return 0;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    (void)sockfd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    return -1;
}

pid_t tcgetpgrp(int fd) {
    (void)fd;
    return getpid();
}

int tcsetpgrp(int fd, pid_t pgrp) {
    (void)fd;
    (void)pgrp;
    return 0;
}

unsigned int alarm(unsigned int seconds) {
    (void)seconds;
    return 0;
}

extern char **environ;

int execvp(const char *file, char *const argv[]) {
    if (!file || !*file) { errno = ENOENT; return -1; }
    // absolute or relative path: exec directly
    if (strchr(file, '/')) {
        int r = execve(file, argv, environ);
        if (r < 0) errno = ENOENT;
        return r;
    }
    const char *path_env = getenv("PATH");
    if (!path_env) path_env = "/bin:/sbin:/usr/bin:/usr/sbin";
    size_t flen = strlen(file);
    char buf[256];
    const char *p = path_env;
    while (*p) {
        const char *end = p;
        while (*end && *end != ':') end++;
        size_t dlen = (size_t)(end - p);
        if (dlen + flen + 2 < sizeof(buf)) {
            if (dlen == 0) {
                buf[0] = '.'; buf[1] = '/';
                memcpy(buf + 2, file, flen + 1);
            } else {
                memcpy(buf, p, dlen);
                buf[dlen] = '/';
                memcpy(buf + dlen + 1, file, flen + 1);
            }
            // probe with access() before exec: execve does a full kernel
            // arg-copy/free cycle on failure which churns the shared heap
            if (access(buf, 0) == 0)
                return execve(buf, argv, environ);
        }
        p = (*end == ':') ? end + 1 : end;
    }
    errno = ENOENT;
    return -1;
}

