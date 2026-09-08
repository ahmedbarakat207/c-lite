#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

extern long syscall(long number, ...);
#define SYS_NANOSLEEP 21

time_t time(time_t *tloc) {
    time_t t = 0;
    if (tloc) *tloc = t;
    return t;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    if (tv) {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    /* No nanosleep syscall in LiteBSD kernel yet (21 is fstat, not nanosleep).
     * Stub: pretend sleep succeeded immediately. */
    (void)req;
    (void)rem;
    return 0;
}

unsigned int sleep(unsigned int seconds) {
    struct timespec req;
    req.tv_sec = (time_t)seconds;
    req.tv_nsec = 0;
    nanosleep(&req, NULL);
    return 0;
}

static struct tm static_tm;

struct tm *gmtime_r(const time_t *timep, struct tm *result) {
    (void)timep;
    if (!result) return NULL;
    result->tm_sec = 0;
    result->tm_min = 0;
    result->tm_hour = 0;
    result->tm_mday = 1;
    result->tm_mon = 0;
    result->tm_year = 70;
    result->tm_wday = 4;
    result->tm_yday = 0;
    result->tm_isdst = 0;
    return result;
}

struct tm *localtime_r(const time_t *timep, struct tm *result) {
    return gmtime_r(timep, result);
}

struct tm *gmtime(const time_t *timep) {
    return gmtime_r(timep, &static_tm);
}

struct tm *localtime(const time_t *timep) {
    return gmtime_r(timep, &static_tm);
}

char *ctime(const time_t *timep) {
    (void)timep;
    return "Thu Jan  1 00:00:00 1970\n";
}

time_t mktime(struct tm *tm) {
    (void)tm;
    return 0;
}

clock_t clock(void) {
    return 0;
}

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm) {
    if (!s || max == 0 || !format || !tm) return 0;
    char *p = s;
    char *end = s + max - 1;

    static const char *wday_name[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char *wday_fullname[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    static const char *mon_name[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static const char *mon_fullname[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    while (*format && p < end) {
        if (*format != '%') {
            *p++ = *format++;
            continue;
        }
        format++;
        char temp[32];
        const char *sub = NULL;
        switch (*format) {
            case '%':
                *p++ = '%';
                format++;
                continue;
            case 'a':
                sub = (tm->tm_wday >= 0 && tm->tm_wday <= 6) ? wday_name[tm->tm_wday] : "?";
                break;
            case 'A':
                sub = (tm->tm_wday >= 0 && tm->tm_wday <= 6) ? wday_fullname[tm->tm_wday] : "?";
                break;
            case 'b':
            case 'h':
                sub = (tm->tm_mon >= 0 && tm->tm_mon <= 11) ? mon_name[tm->tm_mon] : "?";
                break;
            case 'B':
                sub = (tm->tm_mon >= 0 && tm->tm_mon <= 11) ? mon_fullname[tm->tm_mon] : "?";
                break;
            case 'd':
                snprintf(temp, sizeof(temp), "%02d", tm->tm_mday);
                sub = temp;
                break;
            case 'e':
                snprintf(temp, sizeof(temp), "%2d", tm->tm_mday);
                sub = temp;
                break;
            case 'H':
                snprintf(temp, sizeof(temp), "%02d", tm->tm_hour);
                sub = temp;
                break;
            case 'I':
                snprintf(temp, sizeof(temp), "%02d", (tm->tm_hour % 12) ? (tm->tm_hour % 12) : 12);
                sub = temp;
                break;
            case 'm':
                snprintf(temp, sizeof(temp), "%02d", tm->tm_mon + 1);
                sub = temp;
                break;
            case 'M':
                snprintf(temp, sizeof(temp), "%02d", tm->tm_min);
                sub = temp;
                break;
            case 'p':
                sub = (tm->tm_hour >= 12) ? "PM" : "AM";
                break;
            case 'S':
                snprintf(temp, sizeof(temp), "%02d", tm->tm_sec);
                sub = temp;
                break;
            case 'Y':
                snprintf(temp, sizeof(temp), "%d", tm->tm_year + 1900);
                sub = temp;
                break;
            case 'y':
                snprintf(temp, sizeof(temp), "%02d", (tm->tm_year + 1900) % 100);
                sub = temp;
                break;
            default:
                *p++ = *format++;
                continue;
        }
        format++;
        while (*sub && p < end) {
            *p++ = *sub++;
        }
    }
    *p = '\0';
    return (size_t)(p - s);
}
