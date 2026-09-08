#include <signal.h>
#include <unistd.h>
#include <stddef.h>

int sigemptyset(sigset_t *set) {
    if (!set) return -1;
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) return -1;
    *set = ~0UL;
    return 0;
}

int sigaddset(sigset_t *set, int signum) {
    if (!set || signum < 1 || signum >= _NSIG) return -1;
    *set |= (1UL << (signum - 1));
    return 0;
}

int sigdelset(sigset_t *set, int signum) {
    if (!set || signum < 1 || signum >= _NSIG) return -1;
    *set &= ~(1UL << (signum - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signum) {
    if (!set || signum < 1 || signum >= _NSIG) return -1;
    return (*set & (1UL << (signum - 1))) != 0;
}

static sighandler_t handlers[_NSIG];

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum < 1 || signum >= _NSIG) return -1;
    if (oldact) {
        oldact->sa_handler = handlers[signum];
        oldact->sa_flags = 0;
        oldact->sa_mask = 0;
        oldact->sa_restorer = NULL;
    }
    if (act) {
        handlers[signum] = act->sa_handler;
    }
    return 0;
}

sighandler_t signal(int signum, sighandler_t handler) {
    if (signum < 1 || signum >= _NSIG) return SIG_ERR;
    sighandler_t old = handlers[signum];
    handlers[signum] = handler;
    return old;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    (void)how;
    (void)set;
    if (oldset) *oldset = 0;
    return 0;
}

int raise(int sig) {
    return kill(getpid(), sig);
}
