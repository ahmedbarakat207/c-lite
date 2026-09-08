#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "../include/sys/syscall.h"
#include "../include/sys/stat.h"
#include "../include/sys/time.h"
#include "../include/sys/socket.h"
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
    return (int)syscall(sys_execve, (long)path, (long)argv, (long)envp);
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
    (void)fd;
    (void)offset;
    return (void*)syscall(sys_mmap, length, 0, 0);
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
    int ret = (int)syscall(sys_stat, (long)path, (long)buf, 0);
    if (ret < 0) {
        errno = ENOENT;
    }
    return ret;
}

// sys_fstat
int fstat(int fd, struct stat *buf) {
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
    return unlink(path);
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

pid_t vfork(void) {
    return fork();
}

int isatty(int fd) {
    return (fd >= 0 && fd <= 2);
}

int access(const char *pathname, int mode) {
    (void)mode;
    struct stat st;
    return stat(pathname, &st);
}

int lstat(const char *path, struct stat *buf) {
    return stat(path, buf);
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
    (void)filename;
    (void)times;
    return 0;
}

int lutimes(const char *filename, const struct timeval times[2]) {
    (void)filename;
    (void)times;
    return 0;
}

int link(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    return -1;
}

int symlink(const char *target, const char *linkpath) {
    (void)target;
    (void)linkpath;
    return -1;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    (void)pathname;
    (void)buf;
    (void)bufsiz;
    return -1;
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
    if (!file || !*file) return -1;
    if (strchr(file, '/')) {
        return execve(file, argv, environ);
    }
    char path[256];
    // try /bin/file
    size_t flen = strlen(file);
    if (flen + 6 < sizeof(path)) {
        memcpy(path, "/bin/", 5);
        memcpy(path + 5, file, flen + 1);
        if (access(path, 0) == 0) {
            return execve(path, argv, environ);
        }
    }
    // try /file
    if (flen + 2 < sizeof(path)) {
        path[0] = '/';
        memcpy(path + 1, file, flen + 1);
        if (access(path, 0) == 0) {
            return execve(path, argv, environ);
        }
    }
    return execve(file, argv, environ);
}
