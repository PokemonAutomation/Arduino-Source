/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <iostream>
#include "CommonFramework/Logging/Logger.h"
#include "ComputerPrograms/ComputerPrograms_UnitTestRunner.h"

#include "CommandLineTests.h"

#if defined(__APPLE__) || defined(__linux__)
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#endif

using std::cout;
using std::endl;

namespace PokemonAutomation{


#if defined(__APPLE__) || defined(__linux__)
//  On a fatal signal, dump a native backtrace of the crashing thread straight
//  to the stderr file descriptor (fd 2, unbuffered). When stdout/stderr are
//  piped (as in CI) the stdio buffers are lost when the process dies, and the
//  app redirects std::cerr into its logger, so writing to fd 2 directly is the
//  only way the backtrace reliably reaches the CI log.
//
//  backtrace() and backtrace_symbols_fd() are not strictly async-signal-safe,
//  so a re-entrancy guard stops a crash inside the handler from looping: if we
//  re-enter, just die immediately.
static void crash_backtrace_handler(int sig){
    static volatile sig_atomic_t in_handler = 0;
    if (in_handler){
        _exit(128 + sig);
    }
    in_handler = 1;

    const char msg[] = "\n[PA-CRASH] Fatal signal, backtrace of the crashing thread:\n";
    if (write(2, msg, sizeof(msg) - 1) < 0){
        //  Nothing more we can do.
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

static void install_crash_backtrace_handlers(){
    static const int sigs[] = { SIGSEGV, SIGBUS, SIGABRT, SIGILL, SIGFPE };
    for (int sig : sigs){
        signal(sig, crash_backtrace_handler);
    }
}
#else
//  No portable async-signal-safe backtrace API on Windows; the OS crash
//  reports cover it there.
static void install_crash_backtrace_handlers(){}
#endif


int run_command_line_tests(bool augment_color_distortions){
    install_crash_backtrace_handlers();
    //  Keep stdout line-buffered: when piped, stdio defaults to full
    //  buffering, and the last few KB of test output - including the lines
    //  right before a crash - would otherwise be lost in the buffer. An
    //  explicit buffer size is required: with 0, MSVC's ucrt can end up with
    //  a zero-size buffer, which corrupts the next stdout write.
    setvbuf(stdout, nullptr, _IOLBF, 8192);
    cout << "Running parallel unit tests..." << endl;
    ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
    if (runner.run(augment_color_distortions)){
        return 1;
    }
    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
