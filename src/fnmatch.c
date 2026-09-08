#include <fnmatch.h>

int fnmatch(const char *pattern, const char *string, int flags) {
    (void)flags;
    while (*pattern) {
        if (*pattern == '*') {
            while (*pattern == '*') pattern++;
            if (!*pattern) return 0;
            while (*string) {
                if (fnmatch(pattern, string, flags) == 0) return 0;
                string++;
            }
            return FNM_NOMATCH;
        } else if (*pattern == '?' || *pattern == *string) {
            if (!*string) return FNM_NOMATCH;
            pattern++;
            string++;
        } else {
            return FNM_NOMATCH;
        }
    }
    return *string ? FNM_NOMATCH : 0;
}
