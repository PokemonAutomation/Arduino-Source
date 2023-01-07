/*  Video Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCodeReader.h"
#include "PokemonSV/Programs/Multiplayer/PokemonSV_FastCodeEntry.h"
#include "PokemonSV_VideoFastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


VideoFastCodeEntry_Descriptor::VideoFastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:VideoFastCodeEntry",
        STRING_POKEMON + " SV", "Video Fast Code Entry (V-FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/VideoFastCodeEntry.md",
        "Read a 4, 6, or 8 digit link code from someone on your screen and enter it as quickly as possible.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}

VideoFastCodeEntry::VideoFastCodeEntry()
    : SCREEN_WATCHER("Capture Box:", 0.480, 0.510, 0.050, 0.030)
    , SKIP_CONNECT_TO_CONTROLLER(
        "<b>Skip Connect to Controller:</b> If you know your controllers are already connected, you can skip this and save 64 milliseconds.",
        LockWhileRunning::LOCKED,
        false
    )
{
    PA_ADD_OPTION(SCREEN_WATCHER);
    PA_ADD_OPTION(SKIP_CONNECT_TO_CONTROLLER);
    PA_ADD_OPTION(SETTINGS);
}


void VideoFastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    FastCodeEntrySettings settings(SETTINGS);

    std::string code = read_raid_code(env.logger(), SCREEN_WATCHER.screenshot());
    const char* error = enter_code(env, scope, settings, code, !SKIP_CONNECT_TO_CONTROLLER);
    if (error){
        env.log("No valid code found: " + std::string(error), COLOR_RED);
//        throw OperationFailedException(env.logger(), error);
    }

}





}
}
}
