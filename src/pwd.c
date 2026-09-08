#include <pwd.h>
#include <grp.h>
#include <stddef.h>

static struct passwd static_pwd = {
    "root",
    "",
    0,
    0,
    "root",
    "/",
    "/bin/sh"
};

struct passwd *getpwuid(uid_t uid) {
    (void)uid;
    return &static_pwd;
}

struct passwd *getpwnam(const char *name) {
    (void)name;
    return &static_pwd;
}

static char *empty_members[] = { NULL };

static struct group static_grp = {
    "root",
    "",
    0,
    empty_members
};

struct group *getgrgid(gid_t gid) {
    (void)gid;
    return &static_grp;
}

struct group *getgrnam(const char *name) {
    (void)name;
    return &static_grp;
}

int initgroups(const char *user, gid_t group) {
    (void)user;
    (void)group;
    return 0;
}

void endgrent(void) {}
void setgrent(void) {}
struct group *getgrent(void) { return NULL; }
