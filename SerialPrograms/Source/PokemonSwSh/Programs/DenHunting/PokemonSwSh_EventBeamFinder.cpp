/*  Event Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_EventBeamFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


EventBeamFinder_Descriptor::EventBeamFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EventBeamFinder",
        STRING_POKEMON + " SwSh", "Event Beam Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EventBeamFinder.md",
        "Drop wishing pieces until you find an event den.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



EventBeamFinder::EventBeamFinder()
    : WAIT_TIME_IN_DEN(
        "<b>Wait time in Den:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(WAIT_TIME_IN_DEN);
}


void EventBeamFinder::goto_near_den(BotBaseContext& context) const{
    ssf_hold_joystick1(context, true, STICK_CENTER, STICK_MIN, 375);
    pbf_wait(context, 50);
    ssf_press_button1(context, BUTTON_PLUS, 100);
    ssf_press_joystick2(context, true, STICK_MAX, STICK_CENTER, 100, 5);
    ssf_press_button1(context, BUTTON_L, 100);
    ssf_press_button1(context, BUTTON_PLUS, 100);
    ssf_hold_joystick1(context, true, STICK_CENTER, STICK_MIN, 370);
}
void EventBeamFinder::goto_far_den(BotBaseContext& context) const{
    ssf_hold_joystick1(context, true, STICK_CENTER, STICK_MIN, 992);
    pbf_wait(context, 50);
    ssf_press_button1(context, BUTTON_PLUS, 100);
    ssf_press_joystick2(context, true, STICK_MIN, STICK_CENTER, 100, 5);
    ssf_press_button1(context, BUTTON_L, 100);
    ssf_press_button1(context, BUTTON_PLUS, 100);
    ssf_hold_joystick1(context, true, STICK_CENTER, STICK_MIN, 300);
}
void EventBeamFinder::drop_wishing_piece(BotBaseContext& context) const{
    ssf_press_button2(context, BUTTON_A, 200, 10);
    ssf_press_button2(context, BUTTON_A, 150, 10);
    ssf_press_button1(context, BUTTON_A, 5);
    pbf_mash_button(context, BUTTON_B, 500);
    ssf_press_button2(context, BUTTON_A, WAIT_TIME_IN_DEN + 100, 10);
    pbf_mash_button(context, BUTTON_B, 600);
}
void EventBeamFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        pbf_mash_button(context, BUTTON_B, 700);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    bool parity = false;
    while (true){
        //  Fly back to daycare.
        ssf_press_button2(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY, 20);
        pbf_mash_button(context, BUTTON_A, 700);

        //  Goto den.
        if (parity){
            goto_far_den(context);
        }else{
            goto_near_den(context);
        }
        parity = !parity;

        //  Drop wishing piece and see what you get.
        drop_wishing_piece(context);
    }
}



}
}
}
