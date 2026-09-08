# c-lite

A from-scratch C library for LiteBSD. ~3.6k lines of C plus two assembly files, freestanding 32-bit, no POSIX underneath to lean on — every function either talks to the kernel's `int $0x80` gate directly or is faked convincingly enough that BusyBox doesn't notice.

It exists because there was nothing to port: LiteBSD's syscall numbers, its `getdents` layout, even its `_start` contract are custom, so linking newlib or musl was never on the table. What BusyBox 1.36.1 needs, c-lite provides. What it doesn't need is either stubbed to "yes, sure" or left to return -1.

## Build

```
make        # build/build/libc.a + build/crt0.o (+ empty libm.a, see below)
make clean
```

Toolchain is `i686-elf-gcc` / `nasm`, flags tell the story: `-std=gnu11 -ffreestanding -O2 -m32 -fno-pie -fno-stack-protector -fno-builtin -I include`. Output is three artifacts: `libc.a` (14 C objects + `setjmp.o`), `crt0.o`, and `libm.a`, which is an *empty archive* — it only exists so BusyBox's link line has something to point `-lm` at (the LiteBSD busybox patch deletes the `LDLIBS += m` line anyway, belt and suspenders).

Consumers link like this (this is BusyBox's actual incantation via `CONFIG_EXTRA_LDFLAGS`/`CONFIG_EXTRA_LDLIBS`):

```
-nostdlib -Wl,-Ttext,0x8000000 build/crt0.o <objects> -Lbuild -lc
```

`crt0.o` first, text at `0x8000000` (`USER_LOAD_ADDR` — the kernel's ELF loader assumes it).

## Layout

```
src/crt0.asm     _start, argc/argv/envp extraction, .no_args fallback
src/syscall.c    raw gate + every real syscall wrapper + execvp + uid/gid + chmod/chown...
src/stdlib.c     sbrk/malloc/free/calloc/realloc, getenv/setenv, rand, qsort...
src/stdio.c      unbuffered stdio, vsnprintf with width/flags/precision
src/string.c     the usual suspects
src/dirent.c     opendir/readdir over sys_getdents
src/errno.c      global errno + strerror table
src/signal.c     userspace handler table, sigset ops
src/setjmp.asm   6-word jmp_buf
src/time.c       time frozen at epoch, gettimeofday zeros
src/net.c        numeric-IPv4-only getaddrinfo, inet_pton/ntop
src/compat.c     exec family, sessions, sockets stubs, poll/select, termios...
src/pwd.c        everybody is root
src/getopt.c, fnmatch.c, regex.c
```

## The gate

One function does all talking to the kernel:

```c
long syscall(long number, ...) {
    va_list args; ...
    long a1 = va_arg(args, long);
    long a2 = va_arg(args, long);
    long a3 = va_arg(args, long);
    __asm__ volatile("int $0x80" : "=a"(ret)
                     : "a"(number), "b"(a1), "c"(a2), "d"(a3));
}
```

Number in `eax`, up to three args in `ebx/ecx/edx`, return in `eax`. That "up to three" is a real ceiling, not just style — anything needing four arguments can't go through `syscall()` and needs its own stub (nothing currently does). Numbers live in `include/sys/syscall.h` and mirror the kernel table exactly: 1 write … 9 brk, 17 open, 20 stat, 25 getcwd, 26 getdents.

`_exit` fires the syscall then spins on `hlt` in case the kernel ever returns, which it shouldn't — a returned `_exit` means the scheduler is broken.

## Startup

`crt0.asm:_start` clears `ebp`, reads `argc` from `[esp]`, derives `argv = esp+4` and `envp = esp+argc*4+8`, stashes `envp` in `environ`, pushes all three for `main`, then passes `main`'s return value to `exit`.

The quirk is `.no_args`: if `argc <= 0` it calls `main(1, ["sh"], empty_envp)` using static defaults in `.data`. That's not spec behavior, it's survival behavior — LiteBSD's `execve` never sets up the argument stack (fresh stack is just zeroed), so *every* program currently starts through this path. It works because everything is BusyBox multicall and `argv[0]="sh"` gets you a shell.

## malloc

`sbrk` is two `brk` syscalls (query, then set-and-verify). `malloc` keeps an intrusive free list of `block_header_t { size, next, free, magic }` tagged with `BLOCK_MAGIC 0x4C425344`, 8-byte aligns, first-fits, and splits only if the remainder fits another header plus 16 bytes (anti-sliver rule). `free` validates the magic — freeing a wild pointer is silently ignored, not a crash — and coalesces forward, then backward. `calloc`/`realloc` are thin layers on top. No threading, no locks; there are no threads.

## stdio (unbuffered, on purpose)

`stdin/stdout/stderr` are static `FILE`s; `putchar`/`getchar`/`puts` translate straight to 1-byte `read`/`write` syscalls. The `printf` family funnels into one `vsnprintf` that handles dynamic width/precision (`*`), flags, and length modifiers — that plus `errno`-setting wrappers was the `6ad81a9` commit, without which BusyBox `ash`/`hush` misparses half its format strings. There is deliberately no buffering: with the kernel's shared-address-space fork (see LiteBSD README), buffered `FILE` state shared between parent and kid is corruption waiting to happen. Slow and correct beats fast and haunted.

## Directories

LiteBSD's `getdents` (syscall 26) is not Linux's — records are flat `(ino:u32, reclen:u32, name:NUL)`. `opendir` opens the path, mallocs a 4K scratch buffer, slurps the whole directory in *one* syscall, and `readdir` walks `(index → index+reclen)` with a zero-`reclen` tripwire against infinite loops. `rewinddir` just resets the index; `closedir` frees both allocations and closes the fd.

History worth knowing: `opendir(".")` used to rewrite the name to `"/"`, so `ls` in any subdirectory listed root and then failed to stat every entry. Fixed in `76a033c` — `"."` now goes to the kernel, which resolves it against the task's cwd like everything else.

## errno

A plain global `int`, plus a `strerror` table covering the usual suspects. Not thread-local — again, no threads. Conventions are per-call and worth knowing when debugging BusyBox: `open`/`stat`/`unlink`/`chdir` set `ENOENT` on failure, `mkdir` sets `EEXIST` (even when that's not why it failed), `wait4` maps any error to `ECHILD`. `execve` itself sets nothing; the `ENOENT` you see after a failed `execvp` is leftover from its internal `access()` probes (`/bin/file`, then `/file`, then raw).

## Signals, sessions, users

`signal.c` keeps a handler table purely in userspace — the kernel only knows `kill(pid, 9)`. `sigaction`/`signal` record handlers nobody will ever invoke; `sigprocmask` succeeds while doing nothing; `raise` forwards to `kill`. Uids are hardcoded: `getuid` and friends return 0, `setuid`/`setgid` return success, `getpwuid`/`getpwnam`/`getgrgid`/`getgrnam` all return the same static root entries (`root::0:0:root:/:/bin/sh`). Single-user system, single user.

## The honest stubs (compat.c and friends)

Biggest file in the repo (983 lines) and the least proud of itself. Categories:

- **Actually implemented on top of real syscalls:** `execv/execl/execlp/execle/execvpe` (PATH search is `/bin/` then `/`), `setsid/getsid/getpgid/setpgid` (identity — every process is its own session), `sysconf` (a table of plausible constants), `mkstemp/mkdtemp` (pid-hash the `XXXXXX` tail, then `open`/`mkdir`), `realpath` (`getcwd` + collapse of `//` and `/.`, `..` mostly ignored — yes, really).
- **"Yes, dear" stubs that return success:** `chmod/fchmod/chown/mknod/utimes`, `setsockopt`, `tcsetattr/tcgetattr/cfmakeraw`, `setrlimit`, `sysconf`-adjacent bits. BusyBox calls these on startup paths; failing them breaks boot for no benefit.
- **ENOSYS failures:** `socket/bind/listen/accept/connect`, `chroot`, `link/symlink/readlink`, `fchdir`. No network stack, no links, no apology.
- **Fake-ready I/O multiplexing:** `poll` marks every fd `POLLIN|POLLOUT`, `select` returns nonzero. The shell would otherwise block forever waiting for events the kernel never delivers.
- **Odds and ends:** `vsscanf`/`sscanf` (a small hand-rolled parser: `%d%i%u%x%s%c%n`, widths, `l/h` lengths), `strverscmp` (plain `strcmp`), `glob` (returns the pattern itself as the one match — no globbing in the libc, the shell handles it), `usleep` (returns immediately; time is fake anyway).

`time.c` deserves its own warning: `time()` returns 0 and `gettimeofday` zeroes everything — the clock reads epoch, always. And `nanosleep` is a stub that returns success immediately; note its `SYS_NANOSLEEP 21` define is stale (21 is `fstat` in the real table), so if a real nanosleep syscall ever appears this wrapper will need rewiring, not just enabling.

`net.c` resolves numeric IPv4 and the literal `"localhost"` → `127.0.0.1`, nothing else. DNS does not exist here.

## What's missing on purpose

`fork` and `vfork` are the same function (one syscall, no address spaces to copy). `isatty` is `0 <= fd <= 2`. `mmap` ignores every argument except length and hands back `kmalloc`'d kernel memory — protections, flags, and file mappings are fiction. `umask` returns `022` and remembers nothing.

MIT.
