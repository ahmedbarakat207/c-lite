#ifndef _DIRENT_H
#define _DIRENT_H

#include <stdint.h>

struct dirent {
    uint32_t d_ino;
    char d_name[256];
};

typedef struct {
    int fd;
    int index;
    char path[256];
    struct dirent entry;
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif
