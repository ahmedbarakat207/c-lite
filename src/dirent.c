#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct dir_entry_record {
    const char *dir;
    const char *name;
};

static const struct dir_entry_record static_entries[] = {
    { "/", "bin" },
    { "/", "etc" },
    { "/", "usr" },
    { "/", "readme.txt" },
    { "/bin", "sh" },
    { "/bin", "busybox" },
    { "/etc", "passwd" },
    { "/etc", "group" },
    { "/usr", "bin" },
    { NULL, NULL }
};

DIR *opendir(const char *name) {
    if (!name) return NULL;
    if (strcmp(name, ".") == 0) name = "/";
    int fd = open(name, O_RDONLY);
    if (fd < 0) return NULL;
    DIR *dirp = (DIR*)malloc(sizeof(DIR));
    if (!dirp) {
        close(fd);
        return NULL;
    }
    dirp->fd = fd;
    dirp->index = 0;
    size_t len = strlen(name);
    if (len > 1 && name[len - 1] == '/') len--;
    if (len >= sizeof(dirp->path)) len = sizeof(dirp->path) - 1;
    memcpy(dirp->path, name, len);
    dirp->path[len] = '\0';
    return dirp;
}

struct dirent *readdir(DIR *dirp) {
    if (!dirp) return NULL;
    while (static_entries[dirp->index].dir != NULL) {
        int cur = dirp->index++;
        if (strcmp(static_entries[cur].dir, dirp->path) == 0) {
            dirp->entry.d_ino = cur + 1;
            strncpy(dirp->entry.d_name, static_entries[cur].name, sizeof(dirp->entry.d_name) - 1);
            dirp->entry.d_name[sizeof(dirp->entry.d_name) - 1] = '\0';
            return &dirp->entry;
        }
    }
    return NULL;
}

int closedir(DIR *dirp) {
    if (!dirp) return -1;
    close(dirp->fd);
    free(dirp);
    return 0;
}
