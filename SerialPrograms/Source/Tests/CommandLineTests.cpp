/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <iostream>
#include "CommonFramework/Logging/Logger.h"
#include "CommonTools/OCR/OCR_PaddleOCR.h"
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
//  On a fatal signal, dump a native backtrace of the crashing thread straight to stderr
//  (fd 2, unbuffered) so it survives in the CI log even though stdout's buffer is lost when
//  the process segfaults. Then restore the default disposition and re-raise, so the OS still
//  writes its crash report (.ips on macOS) and the exit code still reflects the crash.
static void crash_backtrace_handler(int sig){
    //  Re-entrancy guard: if we crash while handling a signal, just die.
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
    backtrace_symbols_fd(frames, n, 2);  //  writes one symbolized frame per line to fd 2

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
static void install_crash_backtrace_handlers(){}
#endif


int run_command_line_tests(bool augment_color_distortions){
    install_crash_backtrace_handlers();
    //  [ORT-DIAG] Keep stdout line-buffered in CI: when piped, stdio defaults to full
    //  buffering, so the last few KB of test output (and where the crash happened) is lost
    //  in the buffer when the process dies.
    //  8192 bytes: 0 means "implementation-defined" and MSVC's ucrt can end up with a
    //  zero-size buffer, which corrupts the next stdout write (observed 0xC0000409 on the
    //  Windows CI runner).
    setvbuf(stdout, nullptr, _IOLBF, 8192);
    cout << "Running parallel unit tests..." << endl;

    //  [ORT-DIAG] The Mac Intel crash is probabilistic (~60% of CI runs) and happens at
    //  PaddleOCR's ONNX session creation, which normally happens only once per process
    //  (the sessions are cached). To raise the odds of reproducing it in CI, run the full
    //  suite several times, clearing the PaddleOCR cache between runs so the sessions are
    //  re-created each time.
    constexpr int k_repeat_runs = 3;
    for (int run_index = 1; run_index <= k_repeat_runs; ++run_index){
        std::cerr << "[ORT-DIAG] Test suite run " << run_index << " of " << k_repeat_runs << std::endl;
        if (run_index > 1){
            OCR::clear_paddle_ocr_cache();  //  Force PaddleOCR to re-create its ONNX sessions.
        }
        ComputerPrograms::CommandLineUnitTestRunner runner(global_logger_command_line());
        if (runner.run(augment_color_distortions)){
            std::cerr << "[ORT-DIAG] Test suite run " << run_index << " reported failures; stopping."
                      << std::endl;
            return 1;
        }
    }

    cout << "Running parallel unit tests... Done!" << endl;
    return 0;
}



}
