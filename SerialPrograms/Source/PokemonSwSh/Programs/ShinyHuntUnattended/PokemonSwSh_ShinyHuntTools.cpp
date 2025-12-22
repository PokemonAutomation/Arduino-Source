/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
//#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_ShinyHuntTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void run_away_with_lights(ProControllerContext& context){
//    set_leds(context, true);
    pbf_press_dpad(context, DPAD_UP, 80ms, 0ms);
    pbf_press_button(context, BUTTON_A, 80ms, 3000ms);
//    set_leds(context, false);
}
void enter_summary(ProControllerContext& context, bool regi_move_right){
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 0ms);
    pbf_press_button(context, BUTTON_A, 80ms, 2000ms);
    pbf_press_button(context, BUTTON_A, 10, 200);
    if (regi_move_right){
        pbf_move_left_joystick(context, {+1, 0}, 160ms, 240ms);
    }
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 0ms);
    pbf_press_button(context, BUTTON_A, 80ms, 80ms);    //  For Regi, this clears the dialog after running.
}
void close_game_if_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    bool touch_date,
    uint8_t rollback_hours
){
    //  Enter Y-COMM.
    ssf_press_button(context, BUTTON_Y, GameSettings::instance().OPEN_YCOMM_DELAY0, 80ms);

    //  Move the cursor as far away from Link Trade and Surprise Trade as possible.
    //  This is added safety in case connect to internet takes too long.
    pbf_press_dpad(context, DPAD_UP, 5, 0);
    pbf_move_right_joystick(context, {0, +1}, 40ms, 0ms);
    pbf_press_dpad(context, DPAD_RIGHT, 5, 0);

    //  Connect to internet.
    pbf_press_button(context, BUTTON_PLUS, 80ms, 1000ms);

    //  Enter Switch Home.
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    if (touch_date){
        touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    }
    if (rollback_hours > 0){
        rollback_hours_from_home(console, context, rollback_hours, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    }

    //  Enter profile.
    pbf_press_dpad(context, DPAD_UP, 80ms, 80ms);
    pbf_press_button(context, BUTTON_A, 80ms, GameSettings::instance().ENTER_PROFILE_DELAY0);

    //  Back out.
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 80ms);
    pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 80ms);

    //  Close and restart game.
    close_game_from_home(console, context);
    pbf_press_button(context, BUTTON_HOME, 10, 190);
}


}
}
}

