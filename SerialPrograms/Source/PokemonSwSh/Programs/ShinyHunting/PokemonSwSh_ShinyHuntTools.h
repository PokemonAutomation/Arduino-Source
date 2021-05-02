/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H

#include "Common/Compiler.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "NintendoSwitch/FixedInterval.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

static void run_away_with_lights(const BotBaseContext& context){
    set_leds(context, true);
    pbf_press_dpad(context, DPAD_UP, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 3 * TICKS_PER_SECOND);
    set_leds(context, false);
}
static void enter_summary(const BotBaseContext& context, bool regi_move_right){
    pbf_press_dpad(context, DPAD_DOWN, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_A, 10, 200);
    if (regi_move_right){
        pbf_move_left_joystick(context, 255, 128, 20, 30);
    }
    pbf_press_dpad(context, DPAD_DOWN, 10, 0);
    pbf_press_button(context, BUTTON_A, 10, 10);    //  For Regi, this clears the dialog after running.
}
static void close_game_if_overworld(const BotBaseContext& context, bool touch_date, uint8_t rollback_hours){
    //  Enter Y-COMM.
    ssf_press_button2(context, BUTTON_Y, OPEN_YCOMM_DELAY, 10);

    //  Move the cursor as far away from Link Trade and Surprise Trade as possible.
    //  This is added safety in case connect to internet takes too long.
    pbf_press_dpad(context, DPAD_UP, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_press_dpad(context, DPAD_RIGHT, 5, 0);

    //  Connect to internet.
    pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);

    //  Enter Switch Home.
    pbf_press_button(context, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    if (touch_date){
        touch_date_from_home(context, SETTINGS_TO_HOME_DELAY);
    }
    if (rollback_hours > 0){
        rollback_hours_from_home(context, rollback_hours, SETTINGS_TO_HOME_DELAY);
    }

    //  Enter profile.
    pbf_press_dpad(context, DPAD_UP, 10, 10);
    pbf_press_button(context, BUTTON_A, 10, ENTER_PROFILE_DELAY);

    //  Back out.
    pbf_press_dpad(context, DPAD_LEFT, 10, 10);
    pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
    pbf_press_dpad(context, DPAD_DOWN, 10, 10);

    //  Close and restart game.
    close_game(context);
    pbf_press_button(context, BUTTON_HOME, 10, 190);
}


}
}
}
#endif
