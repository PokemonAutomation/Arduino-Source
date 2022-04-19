/*  Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceRoutines_H
#define PokemonAutomation_CommonFramework_InferenceRoutines_H

#include <functional>
#include <chrono>
#include <vector>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "InferenceCallback.h"

namespace PokemonAutomation{

class BotBaseContext;
class ProgramEnvironment;


//  Wait until one of the "callbacks" are triggered or it times out.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before timeout.
//
//  Exceptions thrown in either the commands or the callbacks will stop
//  everything and will be propagated out of this function.
int wait_until(
    ConsoleHandle& console, BotBaseContext& context,
    WallClock deadline,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
);
inline int wait_until(
    ConsoleHandle& console, BotBaseContext& context,
    std::chrono::milliseconds timeout,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    return wait_until(
        console, context,
        current_time() + timeout,
        callbacks,
        default_video_period, default_audio_period
    );
}


//  Run the specified "command" until either it finishes or one of the
//  "callbacks" are triggered.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 if nothing triggered before command fimished.
//
//  Exceptions thrown in either the commands or the callbacks will stop
//  everything and will be propagated out of this function.
int run_until(
    ConsoleHandle& console, BotBaseContext& context,
    std::function<void(BotBaseContext& context)>&& command,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
);



}
#endif
