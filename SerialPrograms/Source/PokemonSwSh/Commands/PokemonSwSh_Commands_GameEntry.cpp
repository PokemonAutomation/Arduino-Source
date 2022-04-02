/*  Game Entry Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Messages_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact(BotBaseContext& context, bool tolerate_update_menu){
    uint16_t HOME_TO_GAME_DELAY = GameSettings::instance().HOME_TO_GAME_DELAY;
    if (tolerate_update_menu){
        pbf_press_button(context, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_press_dpad(context, DPAD_UP, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(context, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }
}
void resume_game_back_out(BotBaseContext& context, bool tolerate_update_menu, uint16_t mash_B_time){
    uint16_t HOME_TO_GAME_DELAY = GameSettings::instance().HOME_TO_GAME_DELAY;
    if (tolerate_update_menu){
        pbf_press_button(context, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_press_dpad(context, DPAD_UP, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, HOME_TO_GAME_DELAY);
        pbf_mash_button(context, BUTTON_B, mash_B_time);
    }else{
        pbf_press_button(context, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }
}
void resume_game_front_of_den_nowatts(BotBaseContext& context, bool tolerate_update_menu){
    resume_game_back_out(context, tolerate_update_menu, 400);
}

void fast_reset_game(
    BotBaseContext& context,
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
){
    context.issue_request(
        DeviceRequest_fast_reset_game(
            start_game_mash, start_game_wait,
            enter_game_mash, enter_game_wait
        )
    );
}

void reset_game_from_home(BotBaseContext& context, bool tolerate_update_menu){
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && !tolerate_update_menu){
        fast_reset_game(
            context,
            GameSettings::instance().START_GAME_MASH, GameSettings::instance().START_GAME_WAIT,
            GameSettings::instance().ENTER_GAME_MASH, GameSettings::instance().ENTER_GAME_WAIT
        );
        return;
    }

    close_game(context);
    start_game_from_home(context, tolerate_update_menu, 0, 0, false);
}
void settings_to_enter_game(BotBaseContext& context, bool fast){
    if (fast){
        //  100 ticks for the first press isn't enough to finish the animation.
        //  But since the HOME button has delayed effect, we start pressing the 2nd
        //  press before the animation finishes.
        pbf_press_button(context, BUTTON_HOME, 10, 90);
        pbf_press_button(context, BUTTON_HOME, 10, 0);
    }else{
        pbf_press_button(context, BUTTON_HOME, 10, 190);
        pbf_press_button(context, BUTTON_HOME, 10, 0);
    }
}
void settings_to_enter_game_den_lobby(
    BotBaseContext& context,
    bool tolerate_update_menu, bool fast,
    uint16_t enter_switch_pokemon_delay,
    uint16_t exit_switch_pokemon_delay
){
    context.issue_request(
        DeviceRequest_settings_to_enter_game_den_lobby(
            tolerate_update_menu, fast,
            enter_switch_pokemon_delay,
            exit_switch_pokemon_delay
        )
    );
}
void start_game_from_home(BotBaseContext& context, bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save){
    //  Start the game with the specified "game_slot" and "user_slot".
    //  If "game_slot" is zero, it uses whatever the cursor is on.
    //  If "user_slot" is zero, it uses whatever the cursor is on.

    if (game_slot != 0){
        pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(context, BUTTON_A, 5, 35);      //  Choose game
        pbf_press_dpad(context, DPAD_UP, 5, 0);          //  Skip the update window.
    }

    bool START_GAME_REQUIRES_INTERNET = ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET;
    uint16_t START_GAME_MASH = GameSettings::instance().START_GAME_MASH;

    if (!START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(context, BUTTON_A, START_GAME_MASH);
    }else{
        pbf_press_button(context, BUTTON_A, 5, 175);     //  Enter select user menu.
        if (user_slot != 0){
            //  Move to correct user.
            for (uint8_t c = 0; c < 8; c++){
                pbf_press_dpad(context, DPAD_LEFT, 7, 7);
            }
//            pbf_wait(50);
            for (uint8_t c = 1; c < user_slot; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 7, 7);
            }
        }
        pbf_press_button(context, BUTTON_A, 5, 5);       //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = START_GAME_MASH;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(context, BUTTON_ZR, duration);
    }

    pbf_wait(context, GameSettings::instance().START_GAME_WAIT);
    enter_game(context, backup_save, GameSettings::instance().ENTER_GAME_MASH, GameSettings::instance().ENTER_GAME_WAIT);
}

void enter_game(BotBaseContext& context, bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait){
    context.issue_request(
        DeviceRequest_enter_game(backup_save, enter_game_mash, enter_game_wait)
    );
}



int register_message_converters_pokemon_game_entry(){
    register_message_converter(
        PABB_MSG_COMMAND_FAST_RESET_GAME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "fast_reset_game() - ";
            if (body.size() != sizeof(pabb_fast_reset_game)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_fast_reset_game*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", start_game_mash = " << params->start_game_mash;
            ss << ", start_game_wait = " << params->start_game_wait;
            ss << ", enter_game_mash = " << params->enter_game_mash;
            ss << ", enter_game_wait = " << params->enter_game_wait;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY,
        [](const std::string& body){
            std::stringstream ss;
            ss << "settings_to_enter_game_den_lobby() - ";
            if (body.size() != sizeof(pabb_settings_to_enter_game_den_lobby)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_settings_to_enter_game_den_lobby*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", tolerate_update_menu = " << params->tolerate_update_menu;
            ss << ", fast = " << params->fast;
            ss << ", enter_switch_pokemon_delay = " << params->enter_switch_pokemon_delay;
            ss << ", exit_switch_pokemon_delay = " << params->exit_switch_pokemon_delay;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ENTER_GAME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "start_game_from_home() - ";
            if (body.size() != sizeof(pabb_enter_game)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_enter_game*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", backup_save = " << params->backup_save;
            ss << ", enter_game_mash = " << params->enter_game_mash;
            ss << ", enter_game_wait = " << params->enter_game_wait;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShGameEntry = register_message_converters_pokemon_game_entry();


}
}
}

