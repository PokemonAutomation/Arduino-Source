/*  Read Summary Screen
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <optional>
#include <sstream>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome/Inference/PokemonHome_SummaryReader.h"
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

ReadSummaryScreen::ReadSummaryScreen()
    : OT_NAME_LANGUAGE(
        "<b>Original Trainer Name Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , HOME_LANGUAGE(
        "<b>Home Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        LockMode::LOCK_WHILE_RUNNING
    )
{
    PA_ADD_OPTION(OT_NAME_LANGUAGE);
    PA_ADD_OPTION(HOME_LANGUAGE);
}

void ReadSummaryScreen::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    SummaryReader reader;
    VideoOverlaySet overlays(env.console);
    reader.make_overlays(overlays);

    VideoSnapshot screen = env.console.video().snapshot();


    std::string language_of_origin = reader.read_language_of_origin(screen);
    std::string level_text = std::to_string(reader.read_level(env.console, screen));
    std::string ability = reader.read_ability(HOME_LANGUAGE, screen);
    std::string nature = reader.read_nature(HOME_LANGUAGE, screen);

    CollectedPokemonInfo pokemon_info{};
    read_summary_screen(env, context, pokemon_info, OT_NAME_LANGUAGE);

    std::optional<CollectedPokemonInfo> logged_info = pokemon_info;
    std::ostringstream ss;
    ss << logged_info;
    env.log("Summary screen result: " + ss.str());
    env.log("Language of origin: " + language_of_origin);
    env.log("Level: " + level_text);
    env.log("Ability: " + ability);
    env.log("Nature: " + nature);
}

}
}
}
