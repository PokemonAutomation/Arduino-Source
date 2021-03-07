/*  Event Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh_EventBeamFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

EventBeamFinder::EventBeamFinder()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Event Beam Finder",
        "NativePrograms/EventBeamFinder.md",
        "Drop wishing pieces until you find an event den."
    )
    , WAIT_TIME_IN_DEN(
        "<b>Wait time in Den:</b>",
        "5 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&WAIT_TIME_IN_DEN, "WAIT_TIME_IN_DEN");
}


void EventBeamFinder::goto_near_den(void) const{
    ssf_hold_joystick1(true, STICK_CENTER, STICK_MIN, 375);
    pbf_wait(50);
    ssf_press_button1(BUTTON_PLUS, 100);
    ssf_press_joystick2(true, STICK_MAX, STICK_CENTER, 100, 5);
    ssf_press_button1(BUTTON_L, 100);
    ssf_press_button1(BUTTON_PLUS, 100);
    ssf_hold_joystick1(true, STICK_CENTER, STICK_MIN, 370);
}
void EventBeamFinder::goto_far_den(void) const{
    ssf_hold_joystick1(true, STICK_CENTER, STICK_MIN, 992);
    pbf_wait(50);
    ssf_press_button1(BUTTON_PLUS, 100);
    ssf_press_joystick2(true, STICK_MIN, STICK_CENTER, 100, 5);
    ssf_press_button1(BUTTON_L, 100);
    ssf_press_button1(BUTTON_PLUS, 100);
    ssf_hold_joystick1(true, STICK_CENTER, STICK_MIN, 300);
}
void EventBeamFinder::drop_wishing_piece(void) const{
    ssf_press_button2(BUTTON_A, 200, 10);
    ssf_press_button2(BUTTON_A, 150, 10);
    ssf_press_button1(BUTTON_A, 5);
    pbf_mash_button(BUTTON_B, 500);
    ssf_press_button2(BUTTON_A, WAIT_TIME_IN_DEN + 100, 10);
    pbf_mash_button(BUTTON_B, 600);
}
void EventBeamFinder::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    pbf_mash_button(BUTTON_B, 700);

    bool parity = false;
    while (true){
        //  Fly back to daycare.
        ssf_press_button2(BUTTON_X, OVERWORLD_TO_MENU_DELAY, 20);
        pbf_mash_button(BUTTON_A, 700);

        //  Goto den.
        if (parity){
            goto_far_den();
        }else{
            goto_near_den();
        }
        parity = !parity;

        //  Drop wishing piece and see what you get.
        drop_wishing_piece();
    }

    end_program_callback();
    end_program_loop();
}



}
}
}
