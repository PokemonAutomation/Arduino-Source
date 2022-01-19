/*  Visual Inference Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceRoutines_H
#define PokemonAutomation_CommonFramework_VisualInferenceRoutines_H

#include <functional>
#include <chrono>
#include <vector>
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{



//  Wait until one of the "callbacks" are triggered or it times out.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 nothing triggered before timeout.
//
//  Exceptions throw in either the commands or the triggers will stop
//  everything and be passed out of this function.
int wait_until(
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds timeout,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);
int wait_until(
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::system_clock::time_point time_limit,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);
int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::milliseconds timeout,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);
int wait_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::chrono::system_clock::time_point time_limit,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);


//  Run the specified "command" until either it finishes or one of the
//  "callbacks" are triggered.
//
//  Returns:
//      -   The index of the trigger if that's what stopped it.
//      -   -1 nothing triggered before command fimished.
//
//  Exceptions throw in either the commands or the triggers will stop
//  everything and be passed out of this function.
int run_until(
    ProgramEnvironment& env, ConsoleHandle& console,
    std::function<void(const BotBaseContext& context)>&& command,
    std::vector<VisualInferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);



}
#endif
