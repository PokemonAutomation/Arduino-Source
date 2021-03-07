/*  Game Entry Routines
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"

using namespace PokemonAutomation;

void resume_game_no_interact(bool tolerate_update_menu){
    resume_game_no_interact(*global_connection, tolerate_update_menu);
}
void resume_game_no_interact(BotBase& device, bool tolerate_update_menu){
    if (tolerate_update_menu){
        pbf_press_button(device, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
        pbf_press_dpad(device, DPAD_DOWN, 10, 10);
        pbf_press_dpad(device, DPAD_UP, 10, 10);
        pbf_press_button(device, BUTTON_A, 10, HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(device, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }
}

void resume_game_back_out(bool tolerate_update_menu, uint16_t mash_B_time){
    resume_game_back_out(*global_connection, tolerate_update_menu, mash_B_time);
}
void resume_game_back_out(BotBase& device, bool tolerate_update_menu, uint16_t mash_B_time){
    if (tolerate_update_menu){
        pbf_press_button(device, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
        pbf_press_dpad(device, DPAD_DOWN, 10, 10);
        pbf_press_dpad(device, DPAD_UP, 10, 10);
        pbf_press_button(device, BUTTON_A, 10, HOME_TO_GAME_DELAY);
        pbf_mash_button(device, BUTTON_B, mash_B_time);
    }else{
        pbf_press_button(device, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }
}

void resume_game_front_of_den_nowatts(bool tolerate_update_menu){
    resume_game_front_of_den_nowatts(*global_connection, tolerate_update_menu);
}
void resume_game_front_of_den_nowatts(BotBase& device, bool tolerate_update_menu){
    resume_game_back_out(device, tolerate_update_menu, 400);
}

void fast_reset_game(
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
){
    fast_reset_game(
        *global_connection,
        start_game_mash, start_game_wait,
        enter_game_mash, enter_game_wait
    );
}
void fast_reset_game(
    BotBase& device,
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
){
    pabb_fast_reset_game params;
    params.start_game_mash = start_game_mash;
    params.start_game_wait = start_game_wait;
    params.enter_game_mash = enter_game_mash;
    params.enter_game_wait = enter_game_wait;
    device.issue_request<PABB_MSG_COMMAND_FAST_RESET_GAME>(params);
}

void reset_game_from_home(bool tolerate_update_menu, uint8_t user_slot, bool game_slot2){
    reset_game_from_home(*global_connection, tolerate_update_menu, user_slot, game_slot2);
}
void reset_game_from_home(BotBase& device, bool tolerate_update_menu, uint8_t user_slot, bool game_slot2){
#if 0
    pabb_reset_game_from_home params;
    params.tolerate_update_menu = tolerate_update_menu;
    params.user_slot = user_slot;
    params.game_slot2 = game_slot2;
    device.issue_request<PABB_MSG_COMMAND_RESET_GAME_FROM_HOME>(params);
#else
    //  If "user_slot" is 0, start whatever the current user is.
    //  If "game_slot2" is true, start the game in the 2nd slot instead of the 1st slot.

    if (!START_GAME_REQUIRES_INTERNET &&
        !tolerate_update_menu &&
        user_slot == 0 && !game_slot2
    ){
        fast_reset_game(
            device,
            START_GAME_MASH, START_GAME_WAIT,
            ENTER_GAME_MASH, ENTER_GAME_WAIT
        );
        return;
    }

    close_game(device);

    start_game_from_home(
        device,
        tolerate_update_menu,
        game_slot2 ? 2 : 0,
        user_slot,
        false
    );
#endif
}

void settings_to_enter_game(bool fast){
    settings_to_enter_game(*global_connection, fast);
}
void settings_to_enter_game(BotBase& device, bool fast){
    if (fast){
        //  100 ticks for the first press isn't enough to finish the animation.
        //  But since the HOME button has delayed effect, we start pressing the 2nd
        //  press before the animation finishes.
        pbf_press_button(device, BUTTON_HOME, 10, 90);
        pbf_press_button(device, BUTTON_HOME, 10, 0);
    }else{
        pbf_press_button(device, BUTTON_HOME, 10, 190);
        pbf_press_button(device, BUTTON_HOME, 10, 0);
    }
}

void settings_to_enter_game_den_lobby(bool tolerate_update_menu, bool fast){
    settings_to_enter_game_den_lobby(*global_connection, tolerate_update_menu, fast);
}
void settings_to_enter_game_den_lobby(BotBase& device, bool tolerate_update_menu, bool fast){
    pabb_settings_to_enter_game_den_lobby params;
    params.tolerate_update_menu = tolerate_update_menu;
    params.fast = fast;
    device.issue_request<PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY>(params);
}

void start_game_from_home(bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save){
    start_game_from_home(*global_connection, tolerate_update_menu, game_slot, user_slot, backup_save);
}
void start_game_from_home(BotBase& device, bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save){
    //  Start the game with the specified "game_slot" and "user_slot".
    //  If "game_slot" is zero, it uses whatever the cursor is on.
    //  If "user_slot" is zero, it uses whatever the cursor is on.

    if (game_slot != 0){
        pbf_press_button(device, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(device, DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(device, BUTTON_A, 5, 35);      //  Choose game
        pbf_press_dpad(device, DPAD_UP, 5, 0);          //  Skip the update window.
    }

    if (!START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(device, BUTTON_A, START_GAME_MASH);
    }else{
        pbf_press_button(device, BUTTON_A, 5, 175);     //  Enter select user menu.
        if (user_slot != 0){
            //  Move to correct user.
            for (uint8_t c = 0; c < 8; c++){
                pbf_press_dpad(device, DPAD_LEFT, 7, 7);
            }
//            pbf_wait(50);
            for (uint8_t c = 1; c < user_slot; c++){
                pbf_press_dpad(device, DPAD_RIGHT, 7, 7);
            }
        }
        pbf_press_button(device, BUTTON_A, 5, 5);       //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = START_GAME_MASH;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(device, BUTTON_ZR, duration);
    }

    pbf_wait(device, START_GAME_WAIT);
    enter_game(device, backup_save, ENTER_GAME_MASH, ENTER_GAME_WAIT);
}

void enter_game(bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait){
    enter_game(*global_connection, backup_save, enter_game_mash, enter_game_wait);
}
void enter_game(BotBase& device, bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait){
    pabb_enter_game params;
    params.backup_save = backup_save;
    params.enter_game_mash = enter_game_mash;
    params.enter_game_wait = enter_game_wait;
    device.issue_request<PABB_MSG_COMMAND_ENTER_GAME>(params);
}

void close_game(void){
    close_game(*global_connection);
}
void close_game(BotBase& device){
    pabb_close_game params;
    device.issue_request<PABB_MSG_COMMAND_CLOSE_GAME>(params);
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
    register_message_converter(
        PABB_MSG_COMMAND_CLOSE_GAME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "close_game() - ";
            if (body.size() != sizeof(pabb_close_game)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_close_game*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShGameEntry = register_message_converters_pokemon_game_entry();
