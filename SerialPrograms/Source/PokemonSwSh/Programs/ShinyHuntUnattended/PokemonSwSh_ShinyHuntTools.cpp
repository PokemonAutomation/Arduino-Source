/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
//#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_ShinyHuntTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void run_away_with_lights(ProControllerContext& context){
//    set_leds(context, true);
    pbf_press_dpad(context, DPAD_UP, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 3 * TICKS_PER_SECOND);
//    set_leds(context, false);
}
void enter_summary(ProControllerContext& context, bool regi_move_right){
    pbf_press_dpad(context, DPAD_DOWN, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_A, 10, 200);
    if (regi_move_right){
        pbf_move_left_joystick(context, 255, 128, 20, 30);
    }
    pbf_press_dpad(context, DPAD_DOWN, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 10);    //  For Regi, this clears the dialog after running.
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
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_press_dpad(context, DPAD_RIGHT, 5, 0);

    //  Connect to internet.
    pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);

    //  Enter Switch Home.
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    if (touch_date){
        touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    }
    if (rollback_hours > 0){
        rollback_hours_from_home(console, context, rollback_hours, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    }

    //  Enter profile.
    pbf_press_dpad(context, DPAD_UP, 10, 10);
    pbf_press_button(context, BUTTON_A, 80ms, GameSettings::instance().ENTER_PROFILE_DELAY0);

    //  Back out.
    pbf_press_dpad(context, DPAD_LEFT, 10, 10);
    pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
    pbf_press_dpad(context, DPAD_DOWN, 10, 10);

    //  Close and restart game.
    close_game_from_home(console, context);
    pbf_press_button(context, BUTTON_HOME, 10, 190);
}


}
}
}

