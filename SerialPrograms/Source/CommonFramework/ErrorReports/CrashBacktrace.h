/*  Crash Backtrace
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_CrashBacktrace_H
#define PokemonAutomation_CrashBacktrace_H

namespace PokemonAutomation{


//  Install handlers for the fatal signals (SIGSEGV/SIGBUS/SIGABRT/
//  SIGILL/SIGFPE) that write the signal number, the faulting address
//  (where the signal carries one), the crashing thread's id, and a
//  native backtrace of the crashing thread straight to fd 2, then
//  restore the default disposition and re-raise the signal so the OS
//  still does its normal crash handling (crash report + exit code).
//
//  Apple/Linux only; a no-op elsewhere (Windows has its own crash
//  handling in ProgramDumper).
void install_crash_backtrace_handlers();


}
#endif
