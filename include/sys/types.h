#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifndef _PID_T_DECLARED
typedef int pid_t;
#define _PID_T_DECLARED
#endif

#ifndef _OFF_T_DECLARED
typedef long off_t;
#define _OFF_T_DECLARED
#endif

#ifndef _SSIZE_T_DECLARED
typedef long ssize_t;
#define _SSIZE_T_DECLARED
#endif

#ifndef _MODE_T_DECLARED
typedef unsigned int mode_t;
#define _MODE_T_DECLARED
#endif

#ifndef _UID_T_DECLARED
typedef unsigned int uid_t;
#define _UID_T_DECLARED
#endif

#ifndef _GID_T_DECLARED
typedef unsigned int gid_t;
#define _GID_T_DECLARED
#endif

typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int nlink_t;
typedef long time_t;
typedef long suseconds_t;
typedef unsigned int useconds_t;
typedef long blksize_t;
typedef long blkcnt_t;

#endif
