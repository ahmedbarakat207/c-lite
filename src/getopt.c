#include <unistd.h>
#include <string.h>
#include <stddef.h>

char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = '?';

static int optpos = 1;

int getopt(int argc, char * const argv[], const char *optstring) {
    if (optind <= 0) {
        optind = 1;
        optpos = 1;
    }
    if (optind >= argc || !argv[optind] || argv[optind][0] != '-' || argv[optind][1] == '\0') {
        return -1;
    }
    if (argv[optind][0] == '-' && argv[optind][1] == '-' && argv[optind][2] == '\0') {
        optind++;
        return -1;
    }

    char c = argv[optind][optpos];
    const char *p = strchr(optstring, c);
    if (!p || c == ':') {
        optopt = c;
        if (argv[optind][++optpos] == '\0') {
            optind++;
            optpos = 1;
        }
        return '?';
    }

    if (p[1] == ':') {
        if (argv[optind][optpos + 1] != '\0') {
            optarg = &argv[optind][optpos + 1];
            optind++;
            optpos = 1;
        } else if (optind + 1 < argc) {
            optarg = argv[++optind];
            optind++;
            optpos = 1;
        } else {
            optopt = c;
            optind++;
            optpos = 1;
            return (optstring[0] == ':') ? ':' : '?';
        }
    } else {
        optarg = NULL;
        if (argv[optind][++optpos] == '\0') {
            optind++;
            optpos = 1;
        }
    }
    return c;
}
