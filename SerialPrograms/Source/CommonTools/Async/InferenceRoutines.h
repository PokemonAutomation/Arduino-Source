/*  Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_InferenceRoutines_H
#define PokemonAutomation_CommonTools_InferenceRoutines_H

#include <functional>
#include <chrono>
#include <vector>
#include "Common/Cpp/Time.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/InferenceCallback.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

class Cancellable;
class ProgramEnvironment;


//
//  Wait until one of the "stoppers" are cancelled or it times out.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before timeout.
//
//  If a stopper is cancelled due to an exception, that exception
//  will be rethrown out of this function.
//
int wait_until(
    CancellableScope& scope,
    WallClock deadline,
    std::vector<Cancellable*> stoppers
);
inline int wait_until(
    CancellableScope& scope,
    std::chrono::milliseconds timeout,
    std::vector<Cancellable*> stoppers
){
    return wait_until(scope, current_time() + timeout, std::move(stoppers));
}

//
//  Run the specified "command" until either it finishes or one of the
//  "stoppers" is cancelled.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before timeout.
//
//  Exceptions thrown in either the commands or the stoppers will stop
//  everything and will be propagated out of this function.
//
int run_until(
    CancellableScope& scope,
    std::function<void(CancellableScope& scope)>&& command,
    std::vector<Cancellable*> stoppers
);
template <typename ControllerContext>
int run_until(
    ControllerContext& context,
    std::function<void(CancellableScope& scope)>&& command,
    std::vector<Cancellable*> stoppers
){
    return run_until(
        context,
        [&](CancellableScope& scope){
            ControllerContext subcontext(scope, context);
            command(subcontext);
            subcontext.wait_for_all_requests();
        },
        std::move(stoppers)
    );
}



//
//  Wait until one of the "callbacks" are triggered or it times out.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before timeout.
//
//  Exceptions thrown in either the commands or the callbacks will stop
//  everything and will be propagated out of this function.
//
int wait_until(
    VideoStream& stream, CancellableScope& scope,
    WallClock deadline,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
);
inline int wait_until(
    VideoStream& stream, CancellableScope& scope,
    std::chrono::milliseconds timeout,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    return wait_until(
        stream, scope,
        current_time() + timeout,
        callbacks,
        default_video_period, default_audio_period
    );
}

//
//  Run the specified "command" until either it finishes or one of the
//  "callbacks" are triggered.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before command finished.
//
//  Exceptions thrown in either the commands or the callbacks will stop
//  everything and will be propagated out of this function.
//
int run_until(
    VideoStream& stream, CancellableScope& scope,
    std::function<void(CancellableScope& scope)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
);
template <typename ControllerContext>
int run_until(
    VideoStream& stream, ControllerContext& context,
    std::function<void(ControllerContext& context)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    return run_until(
        stream, context,
        [&](CancellableScope& scope){
            ControllerContext subcontext(scope, context);
            command(subcontext);
            subcontext.wait_for_all_requests();
        },
        callbacks,
        default_video_period,
        default_audio_period
    );
}


#if 1
//
//  Same as "run_until()", but will cancel the commands and return if a timeout
//  is reached.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before command finished.
//      -   -2 if timed out. Nothing triggered, and command did not finish.
//
int run_until_with_time_limit(
    ProgramEnvironment& env, VideoStream& stream, CancellableScope& scope,
    WallClock deadline,
    std::function<void(CancellableScope& scope)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period,
    std::chrono::milliseconds default_audio_period
);
#endif





}
#endif
