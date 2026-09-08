#ifndef _DIRENT_H
#define _DIRENT_H

#include <stdint.h>

struct dirent {
    uint32_t d_ino;
    char d_name[256];
};

typedef struct {
    int fd;
    unsigned int index;   // byte offset into buf for next readdir() call 
    unsigned int buflen;  // total valid bytes in buf
    void *buf;            // flat kernel dirent buffer filled by sys_getdents
    char path[256];
    struct dirent entry;
} DIR;

DIR           *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void           rewinddir(DIR *dirp);
int            closedir(DIR *dirp);

#endif
