#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <poll.h>
#include <dirent.h>
#include <sched.h>
#include <locale.h>
#include <glob.h>
#include <ctype.h>

extern long syscall(long number, ...);
extern char **environ;

/* ============ exec family ============ */

int execv(const char *path, char *const argv[]) {
    return execve(path, argv, environ);
}

int execl(const char *path, const char *arg, ...) {
    va_list ap;
    const char *args[64];
    int i = 0;
    va_start(ap, arg);
    args[i++] = arg;
    while (i < 63) {
        const char *a = va_arg(ap, const char*);
        args[i++] = a;
        if (!a) break;
    }
    va_end(ap);
    return execve(path, (char*const*)args, environ);
}

int execlp(const char *file, const char *arg, ...) {
    va_list ap;
    const char *args[64];
    int i = 0;
    va_start(ap, arg);
    args[i++] = arg;
    while (i < 63) {
        const char *a = va_arg(ap, const char*);
        args[i++] = a;
        if (!a) break;
    }
    va_end(ap);
    return execvp(file, (char*const*)args);
}

int execle(const char *path, const char *arg, ...) {
    va_list ap;
    const char *args[64];
    int i = 0;
    va_start(ap, arg);
    args[i++] = arg;
    while (i < 63) {
        const char *a = va_arg(ap, const char*);
        args[i++] = a;
        if (!a) break;
    }
    char *const *envp = va_arg(ap, char *const*);
    va_end(ap);
    return execve(path, (char*const*)args, envp);
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    if (!file || !*file) { errno = ENOENT; return -1; }
    if (strchr(file, '/')) {
        return execve(file, argv, envp);
    }
    char path[256];
    size_t flen = strlen(file);
    if (flen + 6 < sizeof(path)) {
        memcpy(path, "/bin/", 5);
        memcpy(path + 5, file, flen + 1);
        struct stat st;
        if (stat(path, &st) == 0) {
            return execve(path, argv, envp);
        }
    }
    if (flen + 2 < sizeof(path)) {
        path[0] = '/';
        memcpy(path + 1, file, flen + 1);
        struct stat st;
        if (stat(path, &st) == 0) {
            return execve(path, argv, envp);
        }
    }
    return execve(file, argv, envp);
}

/* ============ process/session ============ */

pid_t setsid(void) {
    return getpid();
}

pid_t getsid(pid_t pid) {
    if (pid == 0) return getpid();
    return pid;
}

pid_t getpgrp(void) {
    return getpid();
}

int setpgid(pid_t pid, pid_t pgid) {
    (void)pid; (void)pgid;
    return 0;
}

pid_t getpgid(pid_t pid) {
    if (pid == 0) return getpid();
    return pid;
}

/* ============ uid/gid (single-user: always root, always succeed) ============ */

int setuid(uid_t uid) { (void)uid; return 0; }
int setgid(gid_t gid) { (void)gid; return 0; }
int seteuid(uid_t euid) { (void)euid; return 0; }
int setegid(gid_t egid) { (void)egid; return 0; }
int setreuid(uid_t ruid, uid_t euid) { (void)ruid; (void)euid; return 0; }
int setregid(gid_t rgid, gid_t egid) { (void)rgid; (void)egid; return 0; }

/* ============ fs ============ */

int chroot(const char *path) {
    (void)path;
    errno = ENOSYS;
    return -1;
}

int fchdir(int fd) {
    (void)fd;
    errno = ENOSYS;
    return -1;
}

/* ============ tty ============ */

char *ttyname(int fd) {
    static char buf[32];
    if (ttyname_r(fd, buf, sizeof(buf)) != 0) return NULL;
    return buf;
}

int ttyname_r(int fd, char *buf, size_t buflen) {
    if (fd < 0 || fd > 2) { errno = ENOTTY; return ENOTTY; }
    const char *name = "/dev/console";
    size_t len = strlen(name);
    if (len + 1 > buflen) { errno = ERANGE; return ERANGE; }
    memcpy(buf, name, len + 1);
    return 0;
}

/* ============ hostname ============ */

int gethostname(char *name, size_t len) {
    const char *h = "litebsd";
    size_t hl = strlen(h);
    if (hl + 1 > len) { errno = EINVAL; return -1; }
    memcpy(name, h, hl + 1);
    return 0;
}

int sethostname(const char *name, size_t len) {
    (void)name; (void)len;
    return 0;
}

/* ============ sysconf ============ */

long sysconf(int name) {
    switch (name) {
        case _SC_ARG_MAX: return 131072;
        case _SC_CHILD_MAX: return 32;
        case _SC_CLK_TCK: return 100;
        case _SC_NGROUPS_MAX: return 16;
        case _SC_OPEN_MAX: return 32;
        case _SC_STREAM_MAX: return 16;
        case _SC_TZNAME_MAX: return 32;
        case _SC_JOB_CONTROL: return 1;
        case _SC_SAVED_IDS: return 1;
        case _SC_VERSION: return 200809L;
        case _SC_PAGESIZE: return 4096;
        case _SC_PHYS_PAGES: return 1024;
        case _SC_AVPHYS_PAGES: return 1024;
        case _SC_NPROCESSORS_CONF:
        case _SC_NPROCESSORS_ONLN: return 1;
        case _SC_HOST_NAME_MAX: return 64;
        case _SC_LINE_MAX: return 2048;
        case _SC_RE_DUP_MAX: return 255;
        default: return -1;
    }
}

long pathconf(const char *path, int name) {
    (void)path; (void)name;
    return -1;
}

long fpathconf(int fd, int name) {
    (void)fd; (void)name;
    return -1;
}

int getpagesize(void) {
    return 4096;
}

unsigned int usleep(unsigned int usec) {
    (void)usec;
    return 0;
}

/* ============ mkstemp/mkdtemp/mktemp/realpath ============ */

static unsigned int mktemp_counter = 0;

static void mktemp_replace(char *t) {
    size_t len = strlen(t);
    if (len < 6) return;
    char *xs = t + len - 6;
    for (int i = 0; i < 6; i++) {
        if (xs[i] != 'X') return;
    }
    unsigned int v = (unsigned int)getpid() * 2654435761u + (mktemp_counter++);
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 5; i >= 0; i--) {
        xs[i] = chars[v % 62];
        v /= 62;
    }
}

int mkstemp(char *template) {
    if (!template) { errno = EINVAL; return -1; }
    mktemp_replace(template);
    int fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
    return fd;
}

char *mkdtemp(char *template) {
    if (!template || !*template) { errno = EINVAL; return NULL; }
    mktemp_replace(template);
    if (mkdir(template, 0700) != 0) return NULL;
    return template;
}

char *mktemp(char *template) {
    if (!template) return template;
    mktemp_replace(template);
    return template;
}

char *realpath(const char *path, char *resolved_path) {
    if (!path) { errno = EINVAL; return NULL; }
    char tmp[1024];
    if (path[0] == '/') {
        strncpy(tmp, path, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
    } else {
        char cwd[1024];
        if (!getcwd(cwd, sizeof(cwd))) return NULL;
        size_t cl = strlen(cwd);
        size_t pl = strlen(path);
        if (cl + 1 + pl + 1 > sizeof(tmp)) { errno = ENAMETOOLONG; return NULL; }
        memcpy(tmp, cwd, cl);
        tmp[cl] = '/';
        memcpy(tmp + cl + 1, path, pl + 1);
    }
    /* collapse // and /./ ; ignore /../ for now except simple handling */
    char out[1024];
    size_t oi = 0;
    for (size_t i = 0; tmp[i] && oi + 1 < sizeof(out); ) {
        if (tmp[i] == '/' && tmp[i+1] == '/') { i++; continue; }
        if (tmp[i] == '/' && tmp[i+1] == '.' && (tmp[i+2] == '/' || tmp[i+2] == '\0')) { i += 2; continue; }
        out[oi++] = tmp[i++];
    }
    out[oi] = '\0';
    if (oi == 0) { out[0] = '/'; out[1] = '\0'; oi = 1; }
    if (resolved_path) {
        strcpy(resolved_path, out);
        return resolved_path;
    }
    return strdup(out);
}

/* ============ random ============ */

long random(void) {
    return rand();
}

void srandom(unsigned int seed) {
    srand(seed);
}

char *initstate(unsigned int seed, char *state, size_t n) {
    (void)seed; (void)state; (void)n;
    return NULL;
}

char *setstate(char *state) {
    (void)state;
    return NULL;
}

/* ============ string extras ============ */

void *memrchr(const void *s, int c, size_t n) {
    const unsigned char *p = (const unsigned char*)s;
    for (size_t i = n; i > 0; i--) {
        if (p[i-1] == (unsigned char)c) return (void*)(p + i - 1);
    }
    return NULL;
}

char *stpncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest + (strnlen(src, n) < n ? strnlen(src, n) : n);
}

char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    size_t nl = strlen(needle);
    for (; *haystack; haystack++) {
        if (strncasecmp(haystack, needle, nl) == 0) return (char*)haystack;
    }
    return NULL;
}

char *strsignal(int sig) {
    static char buf[32];
    switch (sig) {
        case SIGHUP: return "Hangup";
        case SIGINT: return "Interrupt";
        case SIGQUIT: return "Quit";
        case SIGILL: return "Illegal instruction";
        case SIGABRT: return "Aborted";
        case SIGKILL: return "Killed";
        case SIGSEGV: return "Segmentation fault";
        case SIGPIPE: return "Broken pipe";
        case SIGALRM: return "Alarm clock";
        case SIGTERM: return "Terminated";
        case SIGCHLD: return "Child exited";
        default:
            snprintf(buf, sizeof(buf), "Signal %d", sig);
            return buf;
    }
}

int strverscmp(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {
    if (needlelen == 0) return (void*)haystack;
    if (haystacklen < needlelen) return NULL;
    const unsigned char *h = haystack;
    const unsigned char *n = needle;
    for (size_t i = 0; i + needlelen <= haystacklen; i++) {
        if (memcmp(h + i, n, needlelen) == 0) return (void*)(h + i);
    }
    return NULL;
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
    if (str) *saveptr = str;
    if (!*saveptr) return NULL;
    char *s = *saveptr;
    s += strspn(s, delim);
    if (!*s) { *saveptr = NULL; return NULL; }
    char *end = s + strcspn(s, delim);
    if (*end) {
        *end = '\0';
        *saveptr = end + 1;
    } else {
        *saveptr = NULL;
    }
    return s;
}

char *strerror_r(int errnum, char *buf, size_t buflen) {
    const char *s = strerror(errnum);
    if (buf && buflen > 0) {
        strncpy(buf, s, buflen - 1);
        buf[buflen - 1] = '\0';
    }
    return buf;
}

/* ============ signal extras ============ */

int sigsuspend(const sigset_t *mask) {
    (void)mask;
    errno = EINTR;
    return -1;
}

int sigpending(sigset_t *set) {
    if (set) *set = 0;
    return 0;
}

int sigwait(const sigset_t *set, int *sig) {
    (void)set;
    if (sig) *sig = SIGINT;
    errno = EINTR;
    return -1;
}

/* ============ termios ============ */

int tcgetattr(int fd, struct termios *termios_p) {
    if (!termios_p) { errno = EINVAL; return -1; }
    if (ioctl(fd, TCGETS, termios_p) == 0) return 0;
    /* not a tty (or old kernel): report a sane cooked default */
    memset(termios_p, 0, sizeof(*termios_p));
    termios_p->c_lflag = ICANON | ECHO;
    termios_p->c_cc[VMIN] = 1;
    return 0;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    unsigned long req;
    if (!termios_p) { errno = EINVAL; return -1; }
    switch (optional_actions) {
        case TCSADRAIN: req = TCSETSW; break;
        case TCSAFLUSH: req = TCSETSF; break;
        default: req = TCSETS; break;
    }
    return ioctl(fd, req, (void*)termios_p);
}

int tcflush(int fd, int queue_selector) {
    (void)queue_selector;
    if (ioctl(fd, TCFLSH, 0) == 0) return 0;
    return 0;
}

int tcsendbreak(int fd, int duration) {
    (void)fd; (void)duration;
    return 0;
}

int tcdrain(int fd) {
    (void)fd;
    return 0;
}

speed_t cfgetispeed(const struct termios *termios_p) {
    (void)termios_p;
    return B9600;
}

speed_t cfgetospeed(const struct termios *termios_p) {
    (void)termios_p;
    return B9600;
}

int cfsetispeed(struct termios *termios_p, speed_t speed) {
    (void)termios_p; (void)speed;
    return 0;
}

int cfsetospeed(struct termios *termios_p, speed_t speed) {
    (void)termios_p; (void)speed;
    return 0;
}

int cfsetspeed(struct termios *termios_p, speed_t speed) {
    (void)termios_p; (void)speed;
    return 0;
}

void cfmakeraw(struct termios *termios_p) {
    if (!termios_p) return;
    termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    termios_p->c_oflag &= ~OPOST;
    termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    termios_p->c_cflag &= ~(CSIZE|PARENB);
    termios_p->c_cflag |= CS8;
}

/* ============ fcntl ============ */

int fcntl(int fd, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    long arg = va_arg(ap, long);
    va_end(ap);
    (void)fd; (void)arg;
    switch (cmd) {
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
            return dup(fd);
        case F_GETFD:
            return 0;
        case F_SETFD:
        case F_GETFL:
        case F_SETFL:
            return 0;
        default:
            return 0;
    }
}

/* ============ sockets (stubs: no network yet, just link + graceful fail) ============ */

int socket(int domain, int type, int protocol) {
    (void)domain; (void)type; (void)protocol;
    errno = ENOSYS;
    return -1;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    (void)sockfd; (void)addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

int listen(int sockfd, int backlog) {
    (void)sockfd; (void)backlog;
    errno = ENOSYS;
    return -1;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd; (void)addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    (void)sockfd; (void)addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd; (void)addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    (void)sockfd; (void)buf; (void)len; (void)flags;
    errno = ENOSYS;
    return -1;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    (void)sockfd; (void)buf; (void)len; (void)flags;
    errno = ENOSYS;
    return -1;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    (void)sockfd; (void)buf; (void)len; (void)flags; (void)dest_addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    (void)sockfd; (void)buf; (void)len; (void)flags; (void)src_addr; (void)addrlen;
    errno = ENOSYS;
    return -1;
}

ssize_t sendmsg(int sockfd, const void *msg, int flags) {
    (void)sockfd; (void)msg; (void)flags;
    errno = ENOSYS;
    return -1;
}

ssize_t recvmsg(int sockfd, void *msg, int flags) {
    (void)sockfd; (void)msg; (void)flags;
    errno = ENOSYS;
    return -1;
}

int shutdown(int sockfd, int how) {
    (void)sockfd; (void)how;
    errno = ENOSYS;
    return -1;
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
    (void)domain; (void)type; (void)protocol;
    if (sv) { sv[0] = -1; sv[1] = -1; }
    /* emulate with pipe for AF_UNIX+SOCK_STREAM */
    if (domain == AF_UNIX || domain == AF_LOCAL) {
        int p[2];
        if (pipe(p) == 0) { sv[0] = p[0]; sv[1] = p[1]; return 0; }
    }
    errno = ENOSYS;
    return -1;
}

/* ============ netdb ============ */

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    (void)service; (void)hints;
    if (!node) return EAI_NONAME;
    /* support numeric IPv4 only */
    struct in_addr addr;
    extern int inet_aton(const char *cp, struct in_addr *inp);
    if (inet_aton(node, &addr)) {
        struct addrinfo *ai = malloc(sizeof(*ai));
        struct sockaddr_in *sin = malloc(sizeof(*sin));
        if (!ai || !sin) { free(ai); free(sin); return EAI_MEMORY; }
        memset(sin, 0, sizeof(*sin));
        sin->sin_family = AF_INET;
        sin->sin_addr = addr;
        memset(ai, 0, sizeof(*ai));
        ai->ai_family = AF_INET;
        ai->ai_socktype = SOCK_STREAM;
        ai->ai_addrlen = sizeof(*sin);
        ai->ai_addr = (struct sockaddr*)sin;
        *res = ai;
        return 0;
    }
    if (strcmp(node, "localhost") == 0) {
        return getaddrinfo("127.0.0.1", service, hints, res);
    }
    return EAI_NONAME;
}

void freeaddrinfo(struct addrinfo *res) {
    while (res) {
        struct addrinfo *next = res->ai_next;
        free(res->ai_addr);
        free(res->ai_canonname);
        free(res);
        res = next;
    }
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags) {
    (void)salen; (void)flags;
    if (!sa) return EAI_FAIL;
    if (sa->sa_family == AF_INET) {
        const struct sockaddr_in *sin = (const struct sockaddr_in*)sa;
        if (host && hostlen > 0) {
            const char *s = inet_ntoa(sin->sin_addr);
            strncpy(host, s, hostlen - 1);
            host[hostlen - 1] = '\0';
        }
        if (serv && servlen > 0) {
            snprintf(serv, servlen, "%u", ntohs(sin->sin_port));
        }
        return 0;
    }
    return EAI_FAMILY;
}

const char *gai_strerror(int errcode) {
    switch (errcode) {
        case EAI_BADFLAGS: return "Invalid flags";
        case EAI_NONAME: return "Name or service not known";
        case EAI_AGAIN: return "Temporary failure in name resolution";
        case EAI_FAIL: return "Non-recoverable failure in name resolution";
        case EAI_FAMILY: return "Address family not supported";
        case EAI_SOCKTYPE: return "Socket type not supported";
        case EAI_SERVICE: return "Service not supported";
        case EAI_MEMORY: return "Memory allocation failure";
        case EAI_SYSTEM: return "System error";
        case EAI_OVERFLOW: return "Argument buffer overflow";
        default: return "Unknown error";
    }
}

/* ============ arpa/inet extras ============ */

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
    if (af == AF_INET) {
        if (size < INET_ADDRSTRLEN) { errno = ENOSPC; return NULL; }
        const unsigned char *p = src;
        snprintf(dst, size, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
        return dst;
    }
    errno = EAFNOSUPPORT;
    return NULL;
}

int inet_pton(int af, const char *src, void *dst) {
    if (af == AF_INET) {
        struct in_addr addr;
        extern int inet_aton(const char *cp, struct in_addr *inp);
        if (!inet_aton(src, &addr)) return 0;
        memcpy(dst, &addr, sizeof(addr));
        return 1;
    }
    errno = EAFNOSUPPORT;
    return -1;
}

/* ============ poll/select ============ */

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    long r = syscall(sys_poll, (long)fds, (long)nfds, (long)timeout);
    if (r >= 0) return (int)r;
    /* pre-poll kernel: pretend all fds are ready; avoids blocking forever in shell */
    if (!fds) { errno = EFAULT; return -1; }
    for (nfds_t i = 0; i < nfds; i++) {
        fds[i].revents = fds[i].events & (POLLIN | POLLOUT);
        if (fds[i].revents == 0) fds[i].revents = POLLIN;
    }
    return (int)nfds;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    (void)nfds; (void)exceptfds; (void)timeout;
    int n = 0;
    if (readfds) n++;
    if (writefds) n++;
    return n ? n : 1;
}

/* ============ net/if ============ */

unsigned int if_nametoindex(const char *ifname) {
    (void)ifname;
    return 0;
}

char *if_indextoname(unsigned int ifindex, char *ifname) {
    (void)ifindex; (void)ifname;
    errno = ENOSYS;
    return NULL;
}

/* ============ sched ============ */

int sched_yield(void) { return 0; }
int sched_setparam(pid_t pid, const struct sched_param *param) { (void)pid; (void)param; return 0; }
int sched_getparam(pid_t pid, struct sched_param *param) { (void)pid; if (param) param->sched_priority = 0; return 0; }
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param) { (void)pid; (void)policy; (void)param; return 0; }
int sched_getscheduler(pid_t pid) { (void)pid; return SCHED_OTHER; }

/* ============ locale ============ */

char *setlocale(int category, const char *locale) {
    (void)category; (void)locale;
    return "C";
}

struct lconv;
struct lconv *localeconv(void) {
    return NULL;
}

/* ============ resource ============ */

int getrlimit(int resource, struct rlimit *rlim) {
    (void)resource;
    if (!rlim) { errno = EINVAL; return -1; }
    rlim->rlim_cur = ~0UL;
    rlim->rlim_max = ~0UL;
    return 0;
}

int setrlimit(int resource, const struct rlimit *rlim) {
    (void)resource; (void)rlim;
    return 0;
}

/* ============ misc unistd ============ */

int getpagesize(void);
int pause(void) {
    while (1) { }
    return -1;
}

int sync(void) { return 0; }
int fsync(int fd) { (void)fd; return 0; }
int fdatasync(int fd) { (void)fd; return 0; }

int clearenv(void) {
    if (environ) environ[0] = NULL;
    return 0;
}

int uname(struct utsname *buf) {
    if (!buf) { errno = EFAULT; return -1; }
    if ((long)syscall(sys_uname, (long)buf, 0, 0) == 0) return 0;
    /* pre-uname kernel: report static defaults instead of failing */
    memset(buf, 0, sizeof(*buf));
    strcpy(buf->sysname, "LiteBSD");
    strcpy(buf->nodename, "litebsd");
    strcpy(buf->release, "1.0");
    strcpy(buf->version, "LiteBSD i386");
    strcpy(buf->machine, "i386");
    return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
    (void)tv;
    (void)tz;
    return 0;
}

int glob(const char *pattern, int flags, int (*errfunc)(const char *epath, int eerrno), glob_t *pglob) {
    (void)flags;
    (void)errfunc;
    if (!pattern || !pglob) return GLOB_ABORTED;
    pglob->gl_pathc = 1;
    pglob->gl_pathv = (char **)malloc(2 * sizeof(char *));
    if (!pglob->gl_pathv) return GLOB_NOSPACE;
    pglob->gl_pathv[0] = strdup(pattern);
    pglob->gl_pathv[1] = NULL;
    return 0;
}

void globfree(glob_t *pglob) {
    if (!pglob || !pglob->gl_pathv) return;
    for (size_t i = 0; i < pglob->gl_pathc; i++) {
        if (pglob->gl_pathv[i]) free(pglob->gl_pathv[i]);
    }
    free(pglob->gl_pathv);
    pglob->gl_pathv = NULL;
    pglob->gl_pathc = 0;
}

int vsscanf(const char *str, const char *format, va_list ap) {
    if (!str || !format) return 0;
    int matches = 0;
    const char *s = str;
    const char *f = format;

    while (*f) {
        if (isspace((unsigned char)*f)) {
            while (isspace((unsigned char)*f)) f++;
            while (isspace((unsigned char)*s)) s++;
            continue;
        }

        if (*f != '%') {
            if (*s != *f) break;
            s++;
            f++;
            continue;
        }

        f++; // skip '%'
        if (*f == '%') {
            if (*s != '%') break;
            s++;
            f++;
            continue;
        }

        int suppress = 0;
        if (*f == '*') {
            suppress = 1;
            f++;
        }

        int width = 0;
        while (isdigit((unsigned char)*f)) {
            width = width * 10 + (*f - '0');
            f++;
        }

        int is_long = 0;
        if (*f == 'l') {
            is_long = 1;
            f++;
            if (*f == 'l') {
                is_long = 2;
                f++;
            }
        } else if (*f == 'h') {
            is_long = -1;
            f++;
            if (*f == 'h') {
                is_long = -2;
                f++;
            }
        }

        char conv = *f++;
        if (conv == 'n') {
            if (!suppress) {
                int *ptr = va_arg(ap, int *);
                *ptr = (int)(s - str);
            }
            continue;
        }

        if (conv == 'c') {
            int count = (width > 0) ? width : 1;
            if (!*s) break;
            if (!suppress) {
                char *ptr = va_arg(ap, char *);
                for (int i = 0; i < count && *s; i++) {
                    *ptr++ = *s++;
                }
            } else {
                for (int i = 0; i < count && *s; i++) s++;
            }
            matches++;
            continue;
        }

        while (isspace((unsigned char)*s)) s++;
        if (!*s) break;

        if (conv == 's') {
            if (!suppress) {
                char *ptr = va_arg(ap, char *);
                int read_chars = 0;
                while (*s && !isspace((unsigned char)*s) && (width == 0 || read_chars < width)) {
                    *ptr++ = *s++;
                    read_chars++;
                }
                *ptr = '\0';
            } else {
                int read_chars = 0;
                while (*s && !isspace((unsigned char)*s) && (width == 0 || read_chars < width)) {
                    s++;
                    read_chars++;
                }
            }
            matches++;
            continue;
        }

        if (conv == 'd' || conv == 'i' || conv == 'u' || conv == 'x' || conv == 'X') {
            int base = 10;
            if (conv == 'x' || conv == 'X') base = 16;
            else if (conv == 'i') base = 0;

            char *end;
            long long val = strtoll(s, &end, base);
            if (end == s) break;
            s = end;
            if (!suppress) {
                if (is_long == 2) {
                    long long *ptr = va_arg(ap, long long *);
                    *ptr = val;
                } else if (is_long == 1) {
                    long *ptr = va_arg(ap, long *);
                    *ptr = (long)val;
                } else if (is_long == -1) {
                    short *ptr = va_arg(ap, short *);
                    *ptr = (short)val;
                } else if (is_long == -2) {
                    char *ptr = va_arg(ap, char *);
                    *ptr = (char)val;
                } else {
                    int *ptr = va_arg(ap, int *);
                    *ptr = (int)val;
                }
            }
            matches++;
            continue;
        }
    }
    return matches;
}

int sscanf(const char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsscanf(str, format, ap);
    va_end(ap);
    return ret;
}

char *dirname(char *path) {
    static char buf[1024];
    if (!path || !*path) return ".";
    size_t len = strlen(path);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, path, len);
    buf[len] = '\0';
    // strip trailing slashes
    while (len > 1 && buf[len - 1] == '/') buf[--len] = '\0';
    char *last = strrchr(buf, '/');
    if (!last) return ".";
    if (last == buf) return "/";
    *last = '\0';
    return buf;
}

char *basename(char *path) {
    static char buf[1024];
    if (!path || !*path) return ".";
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/') len--;
    const char *end = path + len;
    const char *start = end;
    while (start > path && *(start - 1) != '/') start--;
    size_t n = (size_t)(end - start);
    if (n == 0) return "/";
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    memcpy(buf, start, n);
    buf[n] = '\0';
    return buf;
}
