/*  Stow-On-Side Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh_DateSpam-StowOnSideFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StowOnSideFarmer_Descriptor::StowOnSideFarmer_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:StowOnSideFarmer",
        "Date Spam: Stow-On-Side Farmer",
        "SwSh-Arduino/wiki/Basic:-DateSpam-StowOnSideFarmer",
        "Farm the Stow-on-Side items dealer.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



StowOnSideFarmer::StowOnSideFarmer(const StowOnSideFarmer_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SKIPS(
        "<b>Number of Purchase Attempts:</b>",
        100000
    )
    , SAVE_ITERATIONS(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        0, 0
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
    m_options.emplace_back(&SKIPS, "SKIPS");
    m_options.emplace_back(&SAVE_ITERATIONS, "SAVE_ITERATIONS");
}

void StowOnSideFarmer::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        home_roll_date_enter_game_autorollback(env.console, &year);
        pbf_mash_button(env.console, BUTTON_B, 90);

        pbf_press_button(env.console, BUTTON_A, 10, 10);
        pbf_mash_button(env.console, BUTTON_ZL, 385);
        pbf_mash_button(env.console, BUTTON_B, 700);

        if (SAVE_ITERATIONS != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS){
                save_count = 0;
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(env.console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY);
                pbf_press_button(env.console, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(env.console, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        pbf_press_button(env.console, BUTTON_HOME, 10, 5);
        pbf_mash_button(env.console, BUTTON_B, GAME_TO_HOME_DELAY_FAST - 15);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}
