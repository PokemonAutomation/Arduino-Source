/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh_DenTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(BotBaseContext& context, uint16_t ENTER_ONLINE_DEN_DELAY, bool watts, bool online){
    if (!online){
        if (!watts){
            ssf_press_button2(context, BUTTON_A, GameSettings::instance().ENTER_OFFLINE_DEN_DELAY, 10);
        }else{
            //  This is the critical den-rolling path. It needs to be fast.
            mash_A(context, GameSettings::instance().COLLECT_WATTS_OFFLINE_DELAY);
            pbf_wait(context, GameSettings::instance().ENTER_OFFLINE_DEN_DELAY);
        }
    }else{
        if (!watts){
            ssf_press_button2(context, BUTTON_A, ENTER_ONLINE_DEN_DELAY, 50);
        }else{
            ssf_press_button2(context, BUTTON_A, GameSettings::instance().COLLECT_WATTS_ONLINE_DELAY, 50);
            ssf_press_button2(context, BUTTON_B, 100, 50);
            ssf_press_button2(context, BUTTON_B, ENTER_ONLINE_DEN_DELAY, 50);
        }
    }
}


void enter_lobby(BotBaseContext& context, uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY, bool online, Catchability catchability){
    if (online){
        switch (catchability){
        case Catchability::ALWAYS_CATCHABLE:
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        case Catchability::MAYBE_UNCATCHABLE:
        case Catchability::ALWAYS_UNCATCHABLE:
            ssf_press_button1(context, BUTTON_A, GameSettings::instance().UNCATCHABLE_PROMPT_DELAY);
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        }
    }

    switch (catchability){
    case Catchability::ALWAYS_CATCHABLE:
        ssf_press_button1(context, BUTTON_A, GameSettings::instance().OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    case Catchability::MAYBE_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, GameSettings::instance().UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, GameSettings::instance().OPEN_LOCAL_DEN_LOBBY_DELAY);

        if (!GameSettings::instance().DODGE_UNCATCHABLE_PROMPT_FAST){
            //  lobby-switch        switch-box
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  lobby-switch        switch-party-red
            ssf_press_button1(context, BUTTON_A, GameSettings::instance().ENTER_SWITCH_POKEMON);
            //  switch-box          switch-confirm
            ssf_press_button1(context, BUTTON_Y, 10);
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  switch-party-blue   switch-confirm
            ssf_press_button1(context, BUTTON_A, GameSettings::instance().EXIT_SWITCH_POKEMON);
            //  lobby-switch        lobby-switch
        }
        return;
    case Catchability::ALWAYS_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, GameSettings::instance().UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, GameSettings::instance().OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    }
}


void roll_den(
    ConsoleHandle& console, BotBaseContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
){
    if (skips > 60){
        skips = 60;
    }
    for (uint8_t c = 0; c < skips; c++){
        enter_den(context, ENTER_ONLINE_DEN_DELAY, c != 0, false);
        enter_lobby(context, OPEN_ONLINE_DEN_LOBBY_DELAY, false, catchability);

        //  Skip forward.
        ssf_press_button2(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST, 10);
        home_to_date_time(context, true, false);
        roll_date_forward_1(context, false);

        //  Enter game
        if (console.video().snapshot()){
            console.log("Entering game using inference...");
            pbf_press_button(context, BUTTON_HOME, 10, 90);
            NintendoSwitch::resume_game_from_home(console, context);
        }else{
            console.log("Entering game without inference...", COLOR_RED);
            settings_to_enter_game_den_lobby(
                context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW, true,
                GameSettings::instance().ENTER_SWITCH_POKEMON, GameSettings::instance().EXIT_SWITCH_POKEMON
            );
        }

        //  Exit Raid
        ssf_press_button2(context, BUTTON_B, 120, 50);
        ssf_press_button2(context, BUTTON_A, GameSettings::instance().REENTER_DEN_DELAY, 50);
    }
}
void rollback_date_from_home(BotBaseContext& context, uint8_t skips){
    if (skips == 0){
        return;
    }
    if (skips > 60){
        skips = 60;
    }
    home_to_date_time(context, true, false);
    roll_date_backward_N(context, skips, false);
//    pbf_wait(5);

    //  Note that it is possible for this return animation to run longer than
    //  "SETTINGS_TO_HOME_DELAY" and swallow a subsequent button press.
    //  Therefore the caller needs to be able to tolerate this.
    ssf_press_button2(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY, 10);
}







}
}
}
