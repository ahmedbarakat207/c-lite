#include "../include/stdio.h"
#include "../include/unistd.h"
#include "../include/string.h"
#include "../include/stdlib.h"
#include "../include/fcntl.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static FILE _stdin_file = { STDIN_FILENO, 0, 0, 0, -1 };
static FILE _stdout_file = { STDOUT_FILENO, 0, 0, 0, -1 };
static FILE _stderr_file = { STDERR_FILENO, 0, 0, 0, -1 };

FILE *stdin = &_stdin_file;
FILE *stdout = &_stdout_file;
FILE *stderr = &_stderr_file;

int putchar(int c) {
    unsigned char ch = (unsigned char)c;
    if (write(STDOUT_FILENO, &ch, 1) == 1) {
        return ch;
    }
    return EOF;
}

int getchar(void) {
    unsigned char ch;
    if (read(STDIN_FILENO, &ch, 1) == 1) {
        return ch;
    }
    return EOF;
}

int puts(const char *s) {
    if (!s) return EOF;
    size_t len = strlen(s);
    if (write(STDOUT_FILENO, s, len) < 0) return EOF;
    if (write(STDOUT_FILENO, "\n", 1) < 0) return EOF;
    return (int)len + 1;
}

static void format_num(char **out, size_t *rem, unsigned long val, int base, int uppercase, int width, char pad, int negative, int left_align) {
    char buf[32];
    int i = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            buf[i++] = digits[val % base];
            val /= base;
        }
    }

    if (negative) {
        buf[i++] = '-';
    }

    int pad_count = (width > i) ? (width - i) : 0;

    if (left_align) {
        while (i-- > 0) {
            if (*rem > 1) {
                *(*out)++ = buf[i];
                (*rem)--;
            }
        }
        while (pad_count-- > 0) {
            if (*rem > 1) {
                *(*out)++ = ' ';
                (*rem)--;
            }
        }
        return;
    }

    if (pad == '0' && negative) {
        if (*rem > 1) {
            *(*out)++ = '-';
            (*rem)--;
        }
        i--; // skip '-'
    }

    while (pad_count-- > 0) {
        if (*rem > 1) {
            *(*out)++ = pad;
            (*rem)--;
        }
    }

    while (i-- > 0) {
        if (*rem > 1) {
            *(*out)++ = buf[i];
            (*rem)--;
        }
    }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    if (!str || size == 0) return 0;

    char *out = str;
    size_t rem = size;

    while (*format && rem > 1) {
        if (*format != '%') {
            *out++ = *format++;
            rem--;
            continue;
        }

        format++; // skip '%'
        if (*format == '\0') break;

        // check for %%
        if (*format == '%') {
            *out++ = '%';
            rem--;
            format++;
            continue;
        }

        // flags
        int left_align = 0;
        char pad = ' ';
        while (*format == '-' || *format == '0' || *format == '+' || *format == ' ' || *format == '#') {
            if (*format == '-') left_align = 1;
            else if (*format == '0') pad = '0';
            format++;
        }
        if (left_align) pad = ' ';

        // width
        int width = 0;
        if (*format == '*') {
            width = va_arg(ap, int);
            if (width < 0) {
                left_align = 1;
                width = -width;
            }
            format++;
        } else {
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
        }

        // precision
        int precision = -1;
        if (*format == '.') {
            format++;
            if (*format == '*') {
                precision = va_arg(ap, int);
                format++;
            } else {
                precision = 0;
                while (*format >= '0' && *format <= '9') {
                    precision = precision * 10 + (*format - '0');
                    format++;
                }
            }
        }

        // length modifiers
        int is_long = 0;
        while (*format == 'l' || *format == 'h' || *format == 'z' || *format == 'j' || *format == 't') {
            if (*format == 'l') is_long++;
            format++;
        }

        switch (*format) {
            case 'c': {
                char c = (char)va_arg(ap, int);
                int pad_count = (width > 1) ? (width - 1) : 0;
                if (!left_align) {
                    while (pad_count-- > 0 && rem > 1) { *out++ = ' '; rem--; }
                }
                if (rem > 1) { *out++ = c; rem--; }
                if (left_align) {
                    while (pad_count-- > 0 && rem > 1) { *out++ = ' '; rem--; }
                }
                break;
            }
            case 's': {
                const char *s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                size_t slen = strlen(s);
                if (precision >= 0 && slen > (size_t)precision) {
                    slen = (size_t)precision;
                }
                int pad_count = (width > (int)slen) ? (width - (int)slen) : 0;
                if (!left_align) {
                    while (pad_count-- > 0 && rem > 1) {
                        *out++ = ' ';
                        rem--;
                    }
                }
                for (size_t i = 0; i < slen && rem > 1; i++) {
                    *out++ = s[i];
                    rem--;
                }
                if (left_align) {
                    while (pad_count-- > 0 && rem > 1) {
                        *out++ = ' ';
                        rem--;
                    }
                }
                break;
            }
            case 'd':
            case 'i': {
                long val = is_long ? va_arg(ap, long) : va_arg(ap, int);
                int negative = 0;
                unsigned long uval;
                if (val < 0) {
                    negative = 1;
                    uval = (unsigned long)-val;
                } else {
                    uval = (unsigned long)val;
                }
                format_num(&out, &rem, uval, 10, 0, width, pad, negative, left_align);
                break;
            }
            case 'u': {
                unsigned long val = is_long ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                format_num(&out, &rem, val, 10, 0, width, pad, 0, left_align);
                break;
            }
            case 'x': {
                unsigned long val = is_long ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                format_num(&out, &rem, val, 16, 0, width, pad, 0, left_align);
                break;
            }
            case 'X': {
                unsigned long val = is_long ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                format_num(&out, &rem, val, 16, 1, width, pad, 0, left_align);
                break;
            }
            case 'o': {
                unsigned long val = is_long ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                format_num(&out, &rem, val, 8, 0, width, pad, 0, left_align);
                break;
            }
            case 'p': {
                void *ptr = va_arg(ap, void*);
                if (rem > 3) {
                    *out++ = '0';
                    *out++ = 'x';
                    rem -= 2;
                }
                format_num(&out, &rem, (uintptr_t)ptr, 16, 0, sizeof(uintptr_t) * 2, '0', 0, 0);
                break;
            }
            default:
                if (rem > 1) {
                    *out++ = *format;
                    rem--;
                }
                break;
        }
        format++;
    }

    *out = '\0';
    return (int)(out - str);
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

int vsprintf(char *str, const char *format, va_list ap) {
    return vsnprintf(str, 1048576, format, ap);
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsprintf(str, format, ap);
    va_end(ap);
    return ret;
}

int vdprintf(int fd, const char *format, va_list ap) {
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap);
    if (len > 0) {
        return write(fd, buf, len);
    }
    return 0;
}

__attribute__((weak)) int dprintf(int fd, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vdprintf(fd, format, ap);
    va_end(ap);
    return ret;
}

int vasprintf(char **strp, const char *format, va_list ap) {
    if (!strp || !format) return -1;
    va_list ap_copy;
    va_copy(ap_copy, ap);
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap_copy);
    va_end(ap_copy);
    if (len < 0) return -1;
    if ((size_t)len < sizeof(buf)) {
        *strp = strdup(buf);
        return *strp ? len : -1;
    }
    char *p = (char *)malloc(len + 1);
    if (!p) return -1;
    va_copy(ap_copy, ap);
    len = vsnprintf(p, len + 1, format, ap_copy);
    va_end(ap_copy);
    *strp = p;
    return len;
}

int asprintf(char **strp, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vasprintf(strp, format, ap);
    va_end(ap);
    return ret;
}

int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vdprintf(STDOUT_FILENO, format, ap);
    va_end(ap);
    return ret;
}

int vfprintf(FILE *stream, const char *format, va_list ap) {
    if (!stream) return -1;
    return vdprintf(stream->fd, format, ap);
}

int fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stream, format, ap);
    va_end(ap);
    return ret;
}

int fputs(const char *s, FILE *stream) {
    if (!s || !stream) return EOF;
    size_t len = strlen(s);
    return write(stream->fd, s, len) >= 0 ? 0 : EOF;
}

int fputc(int c, FILE *stream) {
    if (!stream) return EOF;
    unsigned char ch = (unsigned char)c;
    return write(stream->fd, &ch, 1) == 1 ? ch : EOF;
}

int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

int fgetc(FILE *stream) {
    if (!stream) return EOF;
    if (stream->unget != -1) {
        int c = stream->unget;
        stream->unget = -1;
        return c;
    }
    unsigned char ch;
    if (read(stream->fd, &ch, 1) == 1) {
        return ch;
    }
    stream->eof = 1;
    return EOF;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

char *fgets(char *s, int size, FILE *stream) {
    if (!s || size <= 0 || !stream) return NULL;
    int i = 0;
    while (i < size - 1) {
        int c = fgetc(stream);
        if (c == EOF) {
            if (i == 0) return NULL;
            break;
        }
        s[i++] = (char)c;
        if (c == '\n') break;
    }
    s[i] = '\0';
    return s;
}

int ungetc(int c, FILE *stream) {
    if (!stream || c == EOF) return EOF;
    stream->unget = c;
    stream->eof = 0;
    return c;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || !size || !nmemb || !stream) return 0;
    size_t total = size * nmemb;
    ssize_t n = read(stream->fd, ptr, total);
    if (n <= 0) {
        if (n == 0) stream->eof = 1;
        else stream->error = 1;
        return 0;
    }
    return (size_t)n / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || !size || !nmemb || !stream) return 0;
    size_t total = size * nmemb;
    ssize_t n = write(stream->fd, ptr, total);
    if (n < 0) {
        stream->error = 1;
        return 0;
    }
    return (size_t)n / size;
}

int fflush(FILE *stream) {
    (void)stream;
    return 0;
}

int fclose(FILE *stream) {
    if (!stream) return EOF;
    int ret = close(stream->fd);
    if (stream != stdin && stream != stdout && stream != stderr) {
        free(stream);
    }
    return ret;
}

FILE *fdopen(int fd, const char *mode) {
    (void)mode;
    FILE *f = (FILE*)malloc(sizeof(FILE));
    if (!f) return NULL;
    f->fd = fd;
    f->flags = 0;
    f->error = 0;
    f->eof = 0;
    f->unget = -1;
    return f;
}

FILE *fopen(const char *pathname, const char *mode) {
    int flags = 0;
    if (strchr(mode, '+')) {
        flags = 0x0002; // O_RDWR
    } else if (mode[0] == 'r') {
        flags = 0x0000; // O_RDONLY
    } else if (mode[0] == 'w') {
        flags = 0x0001 | 0x0040 | 0x0200; // O_WRONLY | O_CREAT | O_TRUNC
    } else if (mode[0] == 'a') {
        flags = 0x0001 | 0x0040 | 0x0400; // O_WRONLY | O_CREAT | O_APPEND
    }
    int fd = open(pathname, flags, 0666);
    if (fd < 0) return NULL;
    return fdopen(fd, mode);
}

int fileno(FILE *stream) {
    return stream ? stream->fd : -1;
}

int feof(FILE *stream) {
    return stream ? stream->eof : 1;
}

int ferror(FILE *stream) {
    return stream ? stream->error : 1;
}

void clearerr(FILE *stream) {
    if (stream) {
        stream->error = 0;
        stream->eof = 0;
    }
}

int fseek(FILE *stream, long offset, int whence) {
    if (!stream) return -1;
    stream->unget = -1;
    return lseek(stream->fd, offset, whence) >= 0 ? 0 : -1;
}

int fseeko(FILE *stream, off_t offset, int whence) {
    return fseek(stream, (long)offset, whence);
}

long ftell(FILE *stream) {
    if (!stream) return -1;
    return (long)lseek(stream->fd, 0, SEEK_CUR);
}

off_t ftello(FILE *stream) {
    return (off_t)ftell(stream);
}

void rewind(FILE *stream) {
    if (stream) fseek(stream, 0, SEEK_SET);
}

FILE *freopen(const char *pathname, const char *mode, FILE *stream) {
    if (!stream) return NULL;
    if (stream->fd >= 0) {
        close(stream->fd);
    }
    int flags = 0;
    if (strchr(mode, '+')) {
        flags = 0x0002;
    } else if (mode[0] == 'r') {
        flags = 0x0000;
    } else if (mode[0] == 'w') {
        flags = 0x0001 | 0x0040 | 0x0200;
    } else if (mode[0] == 'a') {
        flags = 0x0001 | 0x0040 | 0x0400;
    }
    int fd = open(pathname, flags, 0666);
    if (fd < 0) return NULL;
    stream->fd = fd;
    stream->flags = 0;
    stream->error = 0;
    stream->eof = 0;
    stream->unget = -1;
    return stream;
}
