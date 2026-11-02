/*  Crash Backtrace
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CrashBacktrace.h"

#if defined(__APPLE__) || defined(__linux__)
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ucontext.h>
#if defined(__linux__) && defined(__x86_64__)
#include <sys/reg.h>
#endif
#include <unistd.h>

namespace PokemonAutomation{

//  On a fatal signal, dump the signal, the faulting address (where the
//  signal carries one), the thread id, and a native backtrace of the
//  crashing thread straight to the stderr file descriptor (fd 2,
//  unbuffered). When stdout/stderr are piped (as in CI) the stdio buffers
//  are lost when the process dies, and the app redirects std::cerr into
//  its logger, so writing to fd 2 directly is the only way the output
//  reliably reaches the CI log.
//
//  backtrace() and backtrace_symbols_fd() are not strictly async-signal-
//  safe, so a re-entrancy guard stops a crash inside the handler from
//  looping: if we re-enter, just die immediately.
static void crash_backtrace_handler(int sig, siginfo_t* info, void* ucontext_raw){

    //  The faulting program counter: the exact instruction that died.
    uintptr_t pc = 0;
    if (ucontext_raw != nullptr){
        ucontext_t* ucontext = (ucontext_t*)ucontext_raw;
        #if defined(__APPLE__)
        //  On Apple, uc_mcontext is a pointer (struct __darwin_mcontext64 *).
        #if defined(__x86_64__)
            pc = ucontext->uc_mcontext->__ss.__rip;
        #elif defined(__aarch64__)
            pc = ucontext->uc_mcontext->__ss.__pc;
        #endif
        #elif defined(__linux__)
        #if defined(__x86_64__)
            pc = (uintptr_t)ucontext->uc_mcontext.gregs[REG_RIP];
        #elif defined(__aarch64__)
            pc = (uintptr_t)ucontext->uc_mcontext.pc;
        #endif
        #endif
    }
    static volatile sig_atomic_t in_handler = 0;
    if (in_handler){
        _exit(128 + sig);
    }
    in_handler = 1;

    //  For SIGSEGV/SIGBUS, si_addr is the faulting virtual address: which
    //  region the bad pointer landed in (stack, heap, a library) narrows
    //  the corruption source a lot. For SIGABRT it is typically null.
    char msg[160];
    int len = snprintf(
        msg,
        sizeof(msg),
        "\n[PA-CRASH] Fatal signal %d, pc 0x%lx, fault address 0x%lx, thread 0x%lx; backtrace:\n",
        sig,
        (unsigned long)pc,
        (sig == SIGSEGV || sig == SIGBUS)
            ? (unsigned long)(uintptr_t)info->si_addr
            : 0UL,
        (unsigned long)(uintptr_t)pthread_self()
    );
    if (len > 0){
        if (write(2, msg, (size_t)len) < 0){
            //  Nothing more we can do.
        }
    }

    void* frames[128];
    int n = backtrace(frames, 128);
    //  Writes one symbolized frame per line to fd 2. glibc declares the
    //  return type as int, Apple's as void; ignore it on both.
    backtrace_symbols_fd(frames, n, 2);

    //  Let the OS do its normal crash handling (crash report + exit code).
    signal(sig, SIG_DFL);
    raise(sig);
}

void install_crash_backtrace_handlers(){
    static const int sigs[] = { SIGSEGV, SIGBUS, SIGABRT, SIGILL, SIGFPE };
    for (int sig : sigs){
        //  SA_SIGINFO so the handler gets the faulting address.
        struct sigaction sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = crash_backtrace_handler;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sigaction(sig, &sa, nullptr);
    }
}

}


#else
namespace PokemonAutomation{
    //  Not supported
    void install_crash_backtrace_handlers(){}
}
#endif
