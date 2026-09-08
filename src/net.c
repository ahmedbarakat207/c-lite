#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int h_errno = 0;

const char *hstrerror(int err) {
    switch (err) {
        case HOST_NOT_FOUND: return "Host not found";
        case TRY_AGAIN: return "Host name lookup failure";
        case NO_RECOVERY: return "Unknown server error";
        case NO_DATA: return "No address associated with name";
        default: return "Unknown resolver error";
    }
}

uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF000000) >> 24) |
           ((hostlong & 0x00FF0000) >> 8) |
           ((hostlong & 0x0000FF00) << 8) |
           ((hostlong & 0x000000FF) << 24);
}

uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF00) >> 8) |
           ((hostshort & 0x00FF) << 8);
}

uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong);
}

uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

int inet_aton(const char *cp, struct in_addr *inp) {
    if (!cp) return 0;
    uint32_t val = 0;
    int base, n;
    char c;
    uint32_t parts[4];
    uint32_t *pp = parts;

    for (;;) {
        base = 10;
        if (*cp == '0') {
            if (*++cp == 'x' || *cp == 'X') {
                base = 16;
                cp++;
            } else {
                base = 8;
            }
        }
        val = 0;
        n = 0;
        while ((c = *cp) != '\0') {
            if (isdigit(c)) {
                val = (val * base) + (c - '0');
                cp++;
                n++;
            } else if (base == 16 && isxdigit(c)) {
                val = (val << 4) | (toupper(c) + 10 - 'A');
                cp++;
                n++;
            } else {
                break;
            }
        }
        if (*cp == '.') {
            if (pp >= parts + 3 || n == 0) return 0;
            *pp++ = val;
            cp++;
        } else {
            break;
        }
    }
    if (*cp != '\0' && !isspace(*cp)) return 0;
    if (n == 0) return 0;
    *pp++ = val;
    n = pp - parts;

    switch (n) {
        case 1:
            break;
        case 2:
            if (parts[1] > 0xffffff) return 0;
            val = (parts[0] << 24) | parts[1];
            break;
        case 3:
            if (parts[2] > 0xffff) return 0;
            val = (parts[0] << 24) | (parts[1] << 16) | parts[2];
            break;
        case 4:
            if ((parts[0] | parts[1] | parts[2] | parts[3]) > 0xff) return 0;
            val = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
            break;
        default:
            return 0;
    }
    if (inp) {
        inp->s_addr = htonl(val);
    }
    return 1;
}

in_addr_t inet_addr(const char *cp) {
    struct in_addr val;
    if (inet_aton(cp, &val)) {
        return val.s_addr;
    }
    return (in_addr_t)(-1);
}

char *inet_ntoa(struct in_addr in) {
    static char buf[18];
    unsigned char *p = (unsigned char *)&in.s_addr;
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
    return buf;
}

static struct hostent s_hostent;
static char s_host_name[64];
static struct in_addr s_host_addr;
static char *s_host_addr_list[2];

struct hostent *gethostbyname(const char *name) {
    if (!name) return NULL;
    if (inet_aton(name, &s_host_addr)) {
        strncpy(s_host_name, name, sizeof(s_host_name) - 1);
        s_host_name[sizeof(s_host_name) - 1] = '\0';
        s_hostent.h_name = s_host_name;
        s_hostent.h_aliases = NULL;
        s_hostent.h_addrtype = AF_INET;
        s_hostent.h_length = sizeof(struct in_addr);
        s_host_addr_list[0] = (char *)&s_host_addr;
        s_host_addr_list[1] = NULL;
        s_hostent.h_addr_list = s_host_addr_list;
        return &s_hostent;
    }
    if (strcmp(name, "localhost") == 0) {
        s_host_addr.s_addr = htonl(INADDR_LOOPBACK);
        strcpy(s_host_name, "localhost");
        s_hostent.h_name = s_host_name;
        s_hostent.h_aliases = NULL;
        s_hostent.h_addrtype = AF_INET;
        s_hostent.h_length = sizeof(struct in_addr);
        s_host_addr_list[0] = (char *)&s_host_addr;
        s_host_addr_list[1] = NULL;
        s_hostent.h_addr_list = s_host_addr_list;
        return &s_hostent;
    }
    h_errno = HOST_NOT_FOUND;
    return NULL;
}

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type) {
    (void)len;
    if (type != AF_INET || !addr) {
        h_errno = HOST_NOT_FOUND;
        return NULL;
    }
    memcpy(&s_host_addr, addr, sizeof(struct in_addr));
    strcpy(s_host_name, "localhost");
    s_hostent.h_name = s_host_name;
    s_hostent.h_aliases = NULL;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(struct in_addr);
    s_host_addr_list[0] = (char *)&s_host_addr;
    s_host_addr_list[1] = NULL;
    s_hostent.h_addr_list = s_host_addr_list;
    return &s_hostent;
}

static struct servent s_servent;
struct servent *getservbyname(const char *name, const char *proto) {
    (void)proto;
    if (!name) return NULL;
    s_servent.s_name = (char *)name;
    s_servent.s_aliases = NULL;
    s_servent.s_port = 0;
    s_servent.s_proto = (char *)proto;
    return &s_servent;
}

struct servent *getservbyport(int port, const char *proto) {
    (void)proto;
    s_servent.s_name = "unknown";
    s_servent.s_aliases = NULL;
    s_servent.s_port = port;
    s_servent.s_proto = (char *)proto;
    return &s_servent;
}
