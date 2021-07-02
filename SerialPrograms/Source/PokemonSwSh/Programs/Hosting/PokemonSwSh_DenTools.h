/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenTools_H
#define PokemonAutomation_PokemonSwSh_DenTools_H

#include "Common/Compiler.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "NintendoSwitch/FixedInterval.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


static void enter_den(const BotBaseContext& context, uint16_t ENTER_ONLINE_DEN_DELAY, bool watts, bool online){
    if (!online){
        if (!watts){
            ssf_press_button2(context, BUTTON_A, ENTER_OFFLINE_DEN_DELAY, 10);
        }else{
            //  This is the critical den-rolling path. It needs to be fast.
            mash_A(context, COLLECT_WATTS_OFFLINE_DELAY);
            pbf_wait(context, ENTER_OFFLINE_DEN_DELAY);
        }
    }else{
        if (!watts){
            ssf_press_button2(context, BUTTON_A, ENTER_ONLINE_DEN_DELAY, 50);
        }else{
            ssf_press_button2(context, BUTTON_A, COLLECT_WATTS_ONLINE_DELAY, 50);
            ssf_press_button2(context, BUTTON_B, 100, 50);
            ssf_press_button2(context, BUTTON_B, ENTER_ONLINE_DEN_DELAY, 50);
        }
    }
}

static void enter_lobby(const BotBaseContext& context, uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY, bool online, Catchability catchability){
    if (online){
        switch (catchability){
        case ALWAYS_CATCHABLE:
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        case MAYBE_UNCATCHABLE:
        case ALWAYS_UNCATCHABLE:
            ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        }
    }

    switch (catchability){
    case ALWAYS_CATCHABLE:
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    case MAYBE_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);

        if (!DODGE_UNCATCHABLE_PROMPT_FAST){
            //  lobby-switch        switch-box
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  lobby-switch        switch-party-red
            ssf_press_button1(context, BUTTON_A, ENTER_SWITCH_POKEMON);
            //  switch-box          switch-confirm
            ssf_press_button1(context, BUTTON_Y, 10);
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  switch-party-blue   switch-confirm
            ssf_press_button1(context, BUTTON_A, EXIT_SWITCH_POKEMON);
            //  lobby-switch        lobby-switch
        }
        return;
    case ALWAYS_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    }
}

static void roll_den(
    const BotBaseContext& context,
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
        ssf_press_button2(context, BUTTON_HOME, GAME_TO_HOME_DELAY_FAST, 10);
        home_to_date_time(context, true, false);
        roll_date_forward_1(context, false);

        //  Enter game
        settings_to_enter_game_den_lobby(
            context,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW, true,
            ENTER_SWITCH_POKEMON, EXIT_SWITCH_POKEMON
        );

        //  Exit Raid
        ssf_press_button2(context, BUTTON_B, 120, 50);
        ssf_press_button2(context, BUTTON_A, REENTER_DEN_DELAY, 50);
    }
}
static void rollback_date_from_home(const BotBaseContext& context, uint8_t skips){
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
    ssf_press_button2(context, BUTTON_HOME, SETTINGS_TO_HOME_DELAY, 10);
}




}
}
}
#endif
