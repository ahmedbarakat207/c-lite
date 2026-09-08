#ifndef _REGEX_H
#define _REGEX_H

#include <stddef.h>

typedef struct {
    size_t re_nsub;
    void *re_comp;
} regex_t;

typedef int regoff_t;

typedef struct {
    regoff_t rm_so;
    regoff_t rm_eo;
} regmatch_t;

#define REG_EXTENDED 1
#define REG_ICASE    2
#define REG_NOSUB    4
#define REG_NEWLINE  8

#define REG_NOMATCH  1
#define REG_BADPAT   2

int regcomp(regex_t *preg, const char *regex, int cflags);
int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
void regfree(regex_t *preg);

#endif
