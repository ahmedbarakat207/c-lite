#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef _PID_T_DECLARED
typedef int pid_t;
#define _PID_T_DECLARED
#endif

#ifndef _OFF_T_DECLARED
typedef long off_t;
#define _OFF_T_DECLARED
#endif

#ifndef _SSIZE_T_DECLARED
typedef long ssize_t;
#define _SSIZE_T_DECLARED
#endif

#include <stdint.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int argc, char * const argv[], const char *optstring);

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

long syscall(long number, ...);

int isatty(int fd);
int access(const char *pathname, int mode);
unsigned int alarm(unsigned int seconds);

int write(int fd, const void *buf, size_t count);
int read(int fd, void *buf, size_t count);
void _exit(int status);
void exit(int status);
pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int getgroups(int size, gid_t list[]);
pid_t fork(void);
pid_t vfork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ...);
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t setsid(void);
pid_t getsid(pid_t pid);
pid_t getpgrp(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t tcgetpgrp(int fd);
int tcsetpgrp(int fd, pid_t pgrp);
int setuid(uid_t uid);
int setgid(gid_t gid);
int seteuid(uid_t euid);
int setegid(gid_t egid);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int chroot(const char *path);
int fchdir(int fd);
char *ttyname(int fd);
int ttyname_r(int fd, char *buf, size_t buflen);
int gethostname(char *name, size_t len);
int sethostname(const char *name, size_t len);
long sysconf(int name);
long pathconf(const char *path, int name);
long fpathconf(int fd, int name);
unsigned int usleep(unsigned int usec);
int getpagesize(void);

#define _SC_ARG_MAX         0
#define _SC_CHILD_MAX       1
#define _SC_CLK_TCK         2
#define _SC_NGROUPS_MAX     3
#define _SC_OPEN_MAX        4
#define _SC_STREAM_MAX      5
#define _SC_TZNAME_MAX      6
#define _SC_JOB_CONTROL     7
#define _SC_SAVED_IDS       8
#define _SC_REALTIME_SIGNALS 9
#define _SC_PRIORITY_SCHEDULING 10
#define _SC_TIMERS          11
#define _SC_ASYNCHRONOUS_IO 12
#define _SC_PRIORITIZED_IO  13
#define _SC_SYNCHRONIZED_IO 14
#define _SC_FSYNC           15
#define _SC_MAPPED_FILES    16
#define _SC_MEMLOCK         17
#define _SC_MEMLOCK_RANGE   18
#define _SC_MEMORY_PROTECTION 19
#define _SC_MESSAGE_PASSING 20
#define _SC_SEMAPHORES      21
#define _SC_SHARED_MEMORY_OBJECTS 22
#define _SC_AIO_LISTIO_MAX  23
#define _SC_AIO_MAX         24
#define _SC_AIO_PRIO_DELTA_MAX 25
#define _SC_DELAYTIMER_MAX  26
#define _SC_MQ_OPEN_MAX     27
#define _SC_MQ_PRIO_MAX     28
#define _SC_VERSION         29
#define _SC_PAGESIZE        30
#define _SC_PAGE_SIZE       _SC_PAGESIZE
#define _SC_RTSIG_MAX       31
#define _SC_SEM_NSEMS_MAX   32
#define _SC_SEM_VALUE_MAX   33
#define _SC_SIGQUEUE_MAX    34
#define _SC_TIMER_MAX       35
#define _SC_BC_BASE_MAX     36
#define _SC_BC_DIM_MAX      37
#define _SC_BC_SCALE_MAX    38
#define _SC_BC_STRING_MAX   39
#define _SC_COLL_WEIGHTS_MAX 40
#define _SC_EXPR_NEST_MAX   41
#define _SC_LINE_MAX        42
#define _SC_RE_DUP_MAX      43
#define _SC_2_VERSION       44
#define _SC_2_C_BIND        45
#define _SC_2_C_DEV         46
#define _SC_2_FORT_DEV      47
#define _SC_2_FORT_RUN      48
#define _SC_2_SW_DEV        49
#define _SC_2_LOCALEDEF     50
#define _SC_PHYS_PAGES      51
#define _SC_AVPHYS_PAGES    52
#define _SC_NPROCESSORS_CONF 53
#define _SC_NPROCESSORS_ONLN 54
#define _SC_HOST_NAME_MAX   55
int chown(const char *pathname, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int lchown(const char *pathname, uid_t owner, gid_t group);
int link(const char *oldpath, const char *newpath);
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
int close(int fd);
int pipe(int pipefd[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *path);
int rmdir(const char *path);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
int brk(void *addr);
void *sbrk(intptr_t increment);
int ftruncate(int fd, off_t length);
int truncate(const char *path, off_t length);

#endif
