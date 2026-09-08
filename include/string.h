#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
void *memcpy(void *dest, const void *src, size_t n);
void *mempcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);

int strcmp(const char *a, const char *b);
int strncmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *stpcpy(char *dest, const char *src);

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

char *strchr(const char *s, int c);
char *strchrnul(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strsep(char **stringp, const char *delim);
char *strtok(char *str, const char *delim);

char *strdup(const char *s);
char *strndup(const char *s, size_t n);
void *memrchr(const void *s, int c, size_t n);
char *stpcpy(char *dest, const char *src);
char *stpncpy(char *dest, const char *src, size_t n);
char *strcasestr(const char *haystack, const char *needle);
char *strsignal(int sig);
int strverscmp(const char *s1, const char *s2);
void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
char *strtok_r(char *str, const char *delim, char **saveptr);

char *strerror(int errnum);
char *strerror_r(int errnum, char *buf, size_t buflen);

#endif
