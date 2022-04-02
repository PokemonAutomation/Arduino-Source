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
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    std::chrono::system_clock::time_point deadline,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);
inline int wait_until(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    std::chrono::milliseconds timeout,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
){
    return wait_until(
        env, context, console,
        std::chrono::system_clock::now() + timeout,
        std::move(callbacks),
        period
    );
}


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
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    std::function<void(BotBaseContext& context)>&& command,
    std::vector<InferenceCallback*>&& callbacks,
    std::chrono::milliseconds period = std::chrono::milliseconds(50)
);



}
#endif
