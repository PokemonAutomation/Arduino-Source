/*  Game Entry Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_Commands_GameEntry.h"
//#include "PokemonSwSh_Messages_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact_old(ProControllerContext& context, bool tolerate_update_menu){
    Milliseconds HOME_TO_GAME_DELAY = GameSettings::instance().HOME_TO_GAME_DELAY0;
    if (tolerate_update_menu){
        pbf_press_button(context, BUTTON_HOME, 80ms, HOME_TO_GAME_DELAY);
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_press_dpad(context, DPAD_UP, 10, 10);
        pbf_press_button(context, BUTTON_A, 80ms, HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(context, BUTTON_HOME, 80ms, HOME_TO_GAME_DELAY);
    }
}
void resume_game_back_out_old(ProControllerContext& context, bool tolerate_update_menu, uint16_t mash_B_time){
    Milliseconds HOME_TO_GAME_DELAY = GameSettings::instance().HOME_TO_GAME_DELAY0;
    if (tolerate_update_menu){
        pbf_press_button(context, BUTTON_HOME, 80ms, HOME_TO_GAME_DELAY);
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_press_dpad(context, DPAD_UP, 10, 10);
        pbf_press_button(context, BUTTON_A, 80ms, HOME_TO_GAME_DELAY);
        pbf_mash_button(context, BUTTON_B, mash_B_time);
    }else{
        pbf_press_button(context, BUTTON_HOME, 80ms, HOME_TO_GAME_DELAY);
    }
}
void resume_game_front_of_den_nowatts(ProControllerContext& context, bool tolerate_update_menu){
    resume_game_back_out_old(context, tolerate_update_menu, 400);
}

void fast_reset_game(
    ProControllerContext& context,
    Milliseconds start_game_mash, Milliseconds start_game_wait,
    Milliseconds enter_game_mash, Milliseconds enter_game_wait
){
    //  Fastest setting. No internet needed and no update menu.
    ssf_mash1_button(context, BUTTON_X, 50);

    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.
    ssf_mash2_button(context, BUTTON_X, BUTTON_A, 3s + start_game_mash);
    ssf_mash1_button(context, BUTTON_X, start_game_wait);

    ssf_mash_AZs(context, enter_game_mash);
    pbf_wait(context, enter_game_wait);
}

void reset_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu
){
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && !tolerate_update_menu){
        fast_reset_game(
            context,
            GameSettings::instance().START_GAME_MASH0,
            GameSettings::instance().START_GAME_WAIT0,
            GameSettings::instance().ENTER_GAME_MASH0,
            GameSettings::instance().ENTER_GAME_WAIT0
        );
        return;
    }

    close_game_from_home(console, context);
    start_game_from_home(context, tolerate_update_menu, 0, 0, false);
}
void settings_to_enter_game(ProControllerContext& context, bool fast){
    if (fast){
        //  100 ticks for the first press isn't enough to finish the animation.
        //  But since the HOME button has delayed effect, we start pressing the 2nd
        //  press before the animation finishes.
        ssf_press_button(context, BUTTON_HOME, 800ms, 160ms);
        ssf_press_button(context, BUTTON_HOME, 160ms, 160ms);
    }else{
        ssf_press_button(context, BUTTON_HOME, 1600ms, 160ms);
        ssf_press_button(context, BUTTON_HOME, 160ms, 160ms);
    }
}
void settings_to_enter_game_den_lobby(
    ProControllerContext& context,
    bool tolerate_update_menu, bool fast,
    Milliseconds enter_switch_pokemon_delay,
    Milliseconds exit_switch_pokemon_delay
){
    settings_to_enter_game(context, fast);
    pbf_wait(context, 90);
    if (tolerate_update_menu){
        //  home                home
//        ssf_press_button2(BUTTON_HOME, 100, 10);
        //  lobby-switch        update-yes
        ssf_press_dpad(context, DPAD_DOWN, 80ms);
        ssf_press_dpad(context, DPAD_UP, 80ms);
        //  lobby-switch        update-start
        ssf_press_button(context, BUTTON_A, enter_switch_pokemon_delay, 80ms);
        //  lobby-select        lobby-switch
        ssf_press_dpad(context, DPAD_LEFT, 80ms);
        //  lobby-select        lobby-switch
        ssf_press_button(context, BUTTON_A, enter_switch_pokemon_delay);
        //  lobby-confirm       lobby-select
        ssf_press_button(context, BUTTON_Y, 80ms);
        ssf_press_dpad(context, DPAD_LEFT, 80ms);
        //  lobby-confirm       lobby-select
        ssf_press_button(context, BUTTON_A, exit_switch_pokemon_delay);
        //  lobby-switch        lobby-switch
    }else{
        pbf_wait(context, 50);
    }
}
void start_game_from_home(
    ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save
){
    //  Start the game with the specified "game_slot" and "user_slot".
    //  If "game_slot" is zero, it uses whatever the cursor is on.
    //  If "user_slot" is zero, it uses whatever the cursor is on.

    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            ssf_press_dpad_ptv(context, DPAD_RIGHT, 80ms);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(context, BUTTON_A, 20, 35);    //  Choose game
        pbf_press_dpad(context, DPAD_UP, 20, 0);        //  Skip the update window.
    }

    bool START_GAME_REQUIRES_INTERNET = ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET;
    Milliseconds START_GAME_MASH = GameSettings::instance().START_GAME_MASH0;

    if (!START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(context, BUTTON_A, START_GAME_MASH);
    }else{
        pbf_press_button(context, BUTTON_A, 20, 160);     //  Enter select user menu.
        if (user_slot != 0){
            //  Move to correct user.
            for (uint8_t c = 0; c < 8; c++){
                ssf_issue_scroll_ptv(context, DPAD_LEFT, 160ms, 160ms);
            }
//            pbf_wait(50);
            for (uint8_t c = 1; c < user_slot; c++){
                ssf_issue_scroll_ptv(context, DPAD_RIGHT, 160ms, 160ms);
            }
        }
        pbf_press_button(context, BUTTON_A, 20, 20);        //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        Milliseconds duration = START_GAME_MASH;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY0;
        }
        pbf_mash_button(context, BUTTON_ZR, duration);
    }

    pbf_wait(context, GameSettings::instance().START_GAME_WAIT0);
    enter_game(
        context,
        backup_save,
        GameSettings::instance().ENTER_GAME_MASH0,
        GameSettings::instance().ENTER_GAME_WAIT0
    );
}

void enter_game(
    ProControllerContext& context,
    bool backup_save,
    Milliseconds enter_game_mash,
    Milliseconds enter_game_wait
){
    if (backup_save){
        pbf_wait(context, enter_game_mash);
        ssf_press_dpad(context, DPAD_UP, 0ms, 80ms);
        ssf_press_button(context, BUTTON_B | BUTTON_X, 1000ms, 80ms);
        ssf_mash_AZs(context, 5000ms);
        if (enter_game_wait > 4s){
            pbf_wait(context, enter_game_wait - 4s);
        }
    }else{
        ssf_mash_AZs(context, enter_game_mash);
        pbf_wait(context, enter_game_wait);
    }
}






}
}
}

