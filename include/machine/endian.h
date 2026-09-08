#ifndef _MACHINE_ENDIAN_H
#define _MACHINE_ENDIAN_H

#include <endian.h>

#define __bswap16(x) __builtin_bswap16(x)
#define __bswap32(x) __builtin_bswap32(x)
#define __bswap64(x) __builtin_bswap64(x)

#endif
