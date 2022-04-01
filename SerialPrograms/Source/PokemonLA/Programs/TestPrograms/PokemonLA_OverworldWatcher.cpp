/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

//#include "CommonFramework/InferenceInfra/VisualInferenceRoutines.h"
#include "CommonFramework/InferenceInfra/VisualInferenceSession.h"
#include "PokemonLA/Inference/Objects/PokemonLA_BubbleDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_QuestMarkDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagDetector.h"
#include "PokemonLA_OverworldWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


OverworldWatcher_Descriptor::OverworldWatcher_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:OverworldWatcher",
        STRING_POKEMON + " LA", "Overworld Watcher",
        "",
        "This is a test program that simply observes the game and labels things of interest. "
        "This program doesn't really do anything.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


OverworldWatcher::OverworldWatcher(const OverworldWatcher_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{
}


void OverworldWatcher::program(SingleSwitchProgramEnvironment& env){
    BubbleDetector bubbles;
    ArcDetector arcs;
    QuestMarkDetector quest_marks;
    FlagDetector flags;

    WhiteObjectWatcher watcher(
        env.console,
        {0, 0, 1, 1},
        {
            {bubbles, false},
            {arcs, false},
            {quest_marks, false},
            {flags, false},
        }
    );

#if 0
    watcher.process_frame(env.console.video().snapshot(), std::chrono::system_clock::now());
#else
    {
        VisualInferenceSession session(env.scope(), env.console, env.console, env.console);
        session += watcher;
        session.run();
    }
#endif

    env.wait_for(std::chrono::seconds(60));
}




}
}
}
