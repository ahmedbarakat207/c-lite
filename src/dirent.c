#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#define GETDENTS_BUFSZ  4096

static inline int _getdents(int fd, void *buf, unsigned int size) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(sys_getdents), "b"(fd), "c"(buf), "d"(size)
        : "memory"
    );
    return ret;
}

DIR *opendir(const char *name) {
    if (!name) return NULL;

    int fd = open(name, O_RDONLY);
    if (fd < 0) return NULL;

    DIR *dirp = (DIR *)malloc(sizeof(DIR));
    if (!dirp) { close(fd); return NULL; }

    dirp->fd    = fd;
    dirp->index = 0;
    dirp->buf   = malloc(GETDENTS_BUFSZ);
    dirp->buflen = 0;

    if (dirp->buf) {
        int n = _getdents(fd, dirp->buf, GETDENTS_BUFSZ);
        if (n > 0) dirp->buflen = (unsigned int)n;
    }

    size_t len = strlen(name);
    if (len > 1 && name[len - 1] == '/') len--;
    if (len >= sizeof(dirp->path)) len = sizeof(dirp->path) - 1;
    memcpy(dirp->path, name, len);
    dirp->path[len] = '\0';

    return dirp;
}

struct dirent *readdir(DIR *dirp) {
    if (!dirp || !dirp->buf) return NULL;

    // Walk the flat buffer of kernel dirent records
    unsigned int pos = dirp->index;
    while (pos + 8 <= dirp->buflen) {
        unsigned int ino    = *((unsigned int *)((char *)dirp->buf + pos));
        unsigned int reclen = *((unsigned int *)((char *)dirp->buf + pos + 4));
        const char  *name   = (const char *)dirp->buf + pos + 8;

        if (reclen == 0) break;      // safety: avoid infinite loop

        dirp->index = pos + reclen;  // advance for next call

        if (name[0] == '\0') {
            pos += reclen;
            continue;
        }

        dirp->entry.d_ino = ino;
        strncpy(dirp->entry.d_name, name, sizeof(dirp->entry.d_name) - 1);
        dirp->entry.d_name[sizeof(dirp->entry.d_name) - 1] = '\0';
        return &dirp->entry;
    }
    return NULL;
}

void rewinddir(DIR *dirp) {
    if (!dirp) return;
    dirp->index = 0;
}

int closedir(DIR *dirp) {
    if (!dirp) return -1;
    if (dirp->buf) free(dirp->buf);
    close(dirp->fd);
    free(dirp);
    return 0;
}
