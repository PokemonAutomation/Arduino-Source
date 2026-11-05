/*  Hang Watchdog
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A watchdog for long-running test phases (e.g., the command-line unit
 *      tests). If no test has COMPLETED for a period of time, it writes
 *      diagnostics (registered state providers + all-thread stack traces)
 *      straight to the stderr file descriptor and terminates the process,
 *      instead of leaving a hung CI runner stuck for hours.
 *
 *      "Zero progress" is defined by the caller's use of
 *      note_test_completed(): it is called each time a test finishes, so the
 *      watchdog fires when the test suite stops completing tests - even if
 *      threads are still spinning or blocked.
 *
 *      The no-progress timeout can be overridden with the environment
 *      variable PA_HANG_WATCHDOG (integer seconds; 0 disables the watchdog).
 *
 */

#ifndef PokemonAutomation_HangWatchdog_H
#define PokemonAutomation_HangWatchdog_H

#include <cstdint>
#include <functional>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{
namespace HangWatchdog{


//  The exit code used when the watchdog terminates the process. Chosen to
//  match the conventional "timed out" code so CI logs are self-explanatory.
constexpr int EXIT_CODE = 124;


//  Start the watchdog with the given no-progress timeout.
//
//  The timeout is clamped/overridden by the PA_HANG_WATCHDOG environment
//  variable if it is set. Idempotent: a second call while running does
//  nothing.
void start(WallDuration no_progress_timeout);

//  Stop the watchdog and wait for its thread to finish. Safe to call if it
//  was never started or was already stopped.
void stop();

//  Call each time a test completes. Resets the no-progress timer.
//  Cheap: one atomic increment and one brief lock.
void note_test_completed();

//  Register a callback that returns text to include in the dump (e.g., the
//  runner's pending/running/completed counts, thread pool states, logger
//  queue depth). Must not block. Cleared by stop().
void set_state_provider(std::function<std::string()>&& provider);

//  Register the calling thread so it can be included in the stack dump on
//  platforms without system-wide thread enumeration (macOS).
//  No-op elsewhere (Linux uses /proc/self/task, Windows a toolhelp snapshot).
void register_thread();

}
}

#endif
