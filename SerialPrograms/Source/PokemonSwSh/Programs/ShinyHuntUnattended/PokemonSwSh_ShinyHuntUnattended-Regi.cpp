/*  ShinyHuntUnattended-Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHunt-Regi.h"
#include "PokemonSwSh_ShinyHuntUnattended-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ShinyHuntUnattendedRegi_Descriptor::ShinyHuntUnattendedRegi_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedRegi",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - Regi",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-Regi.md",
        "Hunt for shiny Regis. Stop when a shiny is found.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntUnattendedRegi::ShinyHuntUnattendedRegi()
    : START_TO_RUN_DELAY(
        "<b>Start to Run Delay:</b><br>This needs to be carefully calibrated.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1990"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CORRECTION_INTERVAL(
        "<b>Correction Interval:</b><br>Periodically leave the building to fix broken lights. Zero disables these corrections.",
        LockWhileRunning::LOCKED,
        20
    )
    , TRANSITION_DELAY(
        "<b>Transition Delay:</b><br>Time to enter/exit the building.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(START_TO_RUN_DELAY);
    PA_ADD_OPTION(REGI_NAME);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(CORRECTION_INTERVAL);
    PA_ADD_OPTION(TRANSITION_DELAY);
}



void ShinyHuntUnattendedRegi::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    uint16_t correct_count = 0;
    for (uint32_t c = 0; ; c++){
        //  Auto-correction.
        bool correct = CORRECTION_INTERVAL > 0 && correct_count >= CORRECTION_INTERVAL;
        move_to_corner(env.console, context, correct, TRANSITION_DELAY);
        if (correct){
            correct_count = 0;
        }

        //  Touch the date.
        if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
            env.log("Touching date to prevent rollover.");
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            touch_date_from_home(context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }


        //  Do the light puzzle.
        run_regi_light_puzzle(env.console, context, REGI_NAME, c);

        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        if (START_TO_RUN_DELAY >= 500){
            //  Extra A press to fix A parity if the lights were messed up.
            pbf_press_button(context, BUTTON_A, 10, 500);
            pbf_press_button(context, BUTTON_A, 10, START_TO_RUN_DELAY - 500);
        }else{
            pbf_press_button(context, BUTTON_A, 10, START_TO_RUN_DELAY);
        }

        //  Run away if not shiny.
        run_away_with_lights(context);

        //  Enter Pokemon menu if shiny.
        enter_summary(context, true);

        correct_count++;
    }

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
//    end_program_callback(env.console);
//    end_program_loop(env.console);
}


}
}
}



