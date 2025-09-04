/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceSession.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_BubbleDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_QuestMarkDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagDetector.h"
#include "PokemonLA_OverworldWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


OverworldWatcher_Descriptor::OverworldWatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:OverworldWatcher",
        STRING_POKEMON + " LA", "Overworld Watcher",
        "",
        "This is a test program that simply observes the game and labels things of interest. "
        "This program doesn't really do anything.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


OverworldWatcher::OverworldWatcher(){}


void OverworldWatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
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

    InferenceSession session(
        context, env.console,
        {{watcher}}
    );
    context.wait_until_cancel();
}




}
}
}
