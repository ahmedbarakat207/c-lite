#ifndef _sys_SYSCALL_H
#define _sys_SYSCALL_H

#define sys_write 1
#define sys_read 2
#define sys_exit 3
#define sys_getpid 4
#define sys_fork 5
#define sys_execve 6
#define sys_wait4 7
#define sys_getppid 8
#define sys_brk 9
#define sys_mmap 10
#define sys_munmap 11
#define sys_pipe 12
#define sys_dup 13
#define sys_dup2 14
#define sys_kill 15
#define sys_ioctl 16
#define sys_open 17
#define sys_close 18
#define sys_lseek 19
#define sys_stat 20
#define sys_fstat 21
#define sys_unlink 22
#define sys_mkdir 23
#define sys_chdir 24
#define sys_getcwd 25
#define sys_getdents 26
#define sys_ftruncate 27
#define sys_poll 28
#define sys_uname 29
#define sys_rename 30
#define sys_rmdir 31
#define sys_symlink 32
#define sys_readlink 33
#define sys_lstat 34
#define sys_truncate 35
#define sys_access 36
#define sys_link 37
#define sys_futimens 38
#define sys_utimens 39

#endif
