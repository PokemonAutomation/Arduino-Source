/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh_DexRecFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

DexRecFinder::DexRecFinder()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "Dex Rec Finder",
        "SerialPrograms/DexRecFinder.md",
        "Search for a " + STRING_POKEDEX + " recommendation by date-spamming."
    )
    , VIEW_TIME(
        "<b>View Time:</b><br>View the " + STRING_POKEDEX + " for this long before continuing.",
        "5 * TICKS_PER_SECOND"
    )
    , BACK_OUT_TIME(
        "<b>Back Out Time:</b><br>Mash B for this long to return to the overworld.",
        "3 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&VIEW_TIME, "VIEW_TIME");
    m_options.emplace_back(&BACK_OUT_TIME, "BACK_OUT_TIME");
}

void DexRecFinder::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);

    while (true){
        home_to_date_time(env.console, true, true);
        neutral_date_skip(env.console);
        settings_to_enter_game(env.console, true);
        pbf_mash_button(env.console, BUTTON_B, 90);
        pbf_press_button(env.console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY - 20);
        pbf_press_button(env.console, BUTTON_A, 10, VIEW_TIME);
        pbf_mash_button(env.console, BUTTON_B, BACK_OUT_TIME);

        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
