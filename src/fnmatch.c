#include <fnmatch.h>
#include <string.h>

// bracket expression matcher: *pp advances past the closing ']' on success.
// returns 1 on match, 0 on no match, -1 on malformed (no closing bracket).
static int match_bracket(const char **pp, int c) {
    const char *p = *pp; // points at '['
    int negate = 0;
    int matched = 0;
    p++;
    if (*p == '!' || *p == '^') {
        negate = 1;
        p++;
    }
    if (*p == '\0') return -1;
    // a ']' first is a literal member, not the closer
    int first = 1;
    while (*p && !(*p == ']' && !first)) {
        first = 0;
        int lo, hi;
        if (*p == '\\' && p[1]) {
            lo = hi = (unsigned char)p[1];
            p += 2;
        } else {
            lo = hi = (unsigned char)*p;
            p++;
        }
        if (*p == '-' && p[1] && p[1] != ']') {
            p++;
            if (*p == '\\' && p[1]) {
                hi = (unsigned char)p[1];
                p += 2;
            } else {
                hi = (unsigned char)*p;
                p++;
            }
        }
        if (c >= lo && c <= hi) matched = 1;
    }
    if (*p != ']') return -1; // unterminated class: treat '[' as literal
    *pp = p + 1;
    return negate ? !matched : matched;
}

// true if the bracket class starting at '[' explicitly lists '.',
// which is the only way to match a leading dot under FNM_PERIOD.
static int class_has_dot(const char *p) {
    p++; // skip '['
    if (*p == '!' || *p == '^') p++;
    if (*p == ']') p++; // literal leading ']' member
    while (*p && *p != ']') {
        if (*p == '\\' && p[1]) {
            if (p[1] == '.') return 1;
            p += 2;
            continue;
        }
        if (*p == '.') return 1;
        // skip range ends (a '.' there also counts, caught above on visit)
        p++;
    }
    return 0;
}

int fnmatch(const char *pattern, const char *string, int flags) {
    int period = (flags & FNM_PERIOD) != 0;
    int noescape = (flags & FNM_NOESCAPE) != 0;
    int at_start = 1; // first char of string (or after '/' with PATHNAME)

    while (*pattern) {
        int pc = (unsigned char)*pattern;

        // backslash escape in pattern: literal match, period rule lifted
        if (pc == '\\' && !noescape && pattern[1]) {
            pattern++;
            pc = (unsigned char)*pattern;
            if (*string != pc) return FNM_NOMATCH;
            if (*string == '\0') return FNM_NOMATCH;
            pattern++;
            string++;
            at_start = 0;
            continue;
        }

        if (pc == '*') {
            while (*pattern == '*') pattern++;
            // a '*' may not consume a leading dot under FNM_PERIOD,
            // not even as its first char
            if (period && at_start && *string == '.') return FNM_NOMATCH;
            if (!*pattern) {
                if ((flags & FNM_PATHNAME) && strchr(string, '/'))
                    return FNM_NOMATCH;
                return 0;
            }
            while (*string) {
                if ((flags & FNM_PATHNAME) && *string == '/') break;
                if (fnmatch(pattern, string, flags) == 0) return 0;
                string++;
            }
            return fnmatch(pattern, string, flags);
        }

        if (pc == '?') {
            if (*string == '\0') return FNM_NOMATCH;
            if (period && at_start && *string == '.') return FNM_NOMATCH;
            if ((flags & FNM_PATHNAME) && *string == '/') return FNM_NOMATCH;
            pattern++;
            string++;
            at_start = 0;
            continue;
        }

        if (pc == '[') {
            const char *psave = pattern;
            int r;
            if (*string == '\0') return FNM_NOMATCH;
            if ((flags & FNM_PATHNAME) && *string == '/') return FNM_NOMATCH;
            if (period && at_start && *string == '.' && !class_has_dot(pattern))
                return FNM_NOMATCH;
            r = match_bracket(&pattern, (unsigned char)*string);
            if (r < 0) {
                // malformed: treat '[' as literal
                pattern = psave;
                if (*string != '[') return FNM_NOMATCH;
                pattern++;
                string++;
                at_start = 0;
                continue;
            }
            if (!r) return FNM_NOMATCH;
            string++;
            at_start = 0;
            continue;
        }

        // literal char (explicit '.', including a leading one, always matches)
        if (*string != pc) return FNM_NOMATCH;
        if ((flags & FNM_PATHNAME) && pc == '/') at_start = 1;
        else at_start = 0;
        pattern++;
        string++;
    }
    return *string ? FNM_NOMATCH : 0;
}
