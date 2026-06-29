/*  Read Summary Screen
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <optional>
#include <sstream>
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome/Programs/PokemonHome_BoxNavigation.h"
#include "PokemonHome_ReadSummaryScreen.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;

ReadSummaryScreen_Descriptor::ReadSummaryScreen_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:ReadSummaryScreen",
        STRING_POKEMON + " Home", "Read Summary Screen",
        "",
        "Read the current Pokemon summary screen in Pokemon Home and log the detected data.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

ReadSummaryScreen::ReadSummaryScreen(){}

void ReadSummaryScreen::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    CollectedPokemonInfo pokemon_info{};
    read_summary_screen(env, context, pokemon_info);

    std::optional<CollectedPokemonInfo> logged_info = pokemon_info;
    std::ostringstream ss;
    ss << logged_info;
    env.log("Summary screen result: " + ss.str());
}

}
}
}
