/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_BeamReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


BeamReset_Descriptor::BeamReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BeamReset",
        STRING_POKEMON + " SwSh", "Beam Reset",
        "Programs/PokemonSwSh/BeamReset.html",
        "Reset a beam until you see a purple beam.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}



BeamReset::BeamReset()
    : DELAY_BEFORE_RESET0(
        "<b>Delay before Reset:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , EXTRA_LINE(
        "<b>Extra Line:</b><br>(German has an extra line of text.)",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(DELAY_BEFORE_RESET0);
    PA_ADD_OPTION(EXTRA_LINE);
}

void BeamReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_front_of_den_nowatts(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }else{
        pbf_press_button(context, BUTTON_B, 40ms, 40ms);
    }

    while (true){
        //  Talk to den.
        pbf_press_button(context, BUTTON_A, 80ms, 3600ms);
        if (EXTRA_LINE){
            pbf_press_button(context, BUTTON_A, 80ms, 2400ms);
        }
        pbf_press_button(context, BUTTON_A, 80ms, 2400ms);

        //  Drop wishing piece.
        pbf_press_button(context, BUTTON_A, 80ms, 560ms);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);

        for (uint16_t c = 0; c < 4; c++){
            pbf_press_button(context, BUTTON_HOME, 80ms, 80ms);
            pbf_press_button(context, BUTTON_HOME, 80ms, 1760ms);
        }
        pbf_wait(context, DELAY_BEFORE_RESET0);

        reset_game_from_home_old(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }
}



}
}
}
