/*  Program Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "ProgramDumper.h"


#if _WIN32 && _MSC_VER
#include "ProgramDumper_Windows.tpp"

#else
//
//  Scratch CI debug: on a fatal signal, dump a symbolized backtrace of
//  the crashing thread to stderr before dying. backtrace_symbols is not
//  async-signal-safe (it allocates), but this scratch build's goal is
//  getting the stack out of the CI log, so that's acceptable.
//
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <unistd.h>

namespace PokemonAutomation{

namespace{

void crash_backtrace(int sig, siginfo_t* info, void* ucontext)
{
    (void)info;
    (void)ucontext;

    char header[128];
    int hlen = snprintf(
        header, sizeof(header),
        "\n=== CRASH: signal %d - backtrace (scratch CI debug) ===\n", sig
    );
    write(2, header, hlen);

    void* frames[128];
    int n = backtrace(frames, 128);
    char** syms = backtrace_symbols(frames, n);
    for (int i = 0; i < n; i++){
        const char* s = syms ? syms[i] : "<unavailable>";
        write(2, s, strlen(s));
        write(2, "\n", 1);
    }
    if (syms){
        free(syms);
    }

    //  Re-die with the default disposition so the crash still registers.
    signal(sig, SIG_DFL);
    raise(sig);
}

}


    void setup_crash_handler(){
        struct sigaction sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = crash_backtrace;
        sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGILL,  &sa, nullptr);
        sigaction(SIGFPE,  &sa, nullptr);
    }
    bool program_dump(Logger* logger, const std::string& filename){
        return false;
    }
}
#endif
