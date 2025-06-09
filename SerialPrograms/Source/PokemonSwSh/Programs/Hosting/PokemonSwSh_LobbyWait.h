/*  Lobby Wait
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LobbyWait_H
#define PokemonAutomation_PokemonSwSh_LobbyWait_H

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


static RaidLobbyState raid_lobby_wait(
    ConsoleHandle& console, ProControllerContext& context,
    bool HOST_ONLINE,
    uint8_t accept_FR_slot,
    Milliseconds lobby_wait_delay
){
    Milliseconds GAME_TO_HOME_DELAY_SAFE = GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0;
    Milliseconds AUTO_FR_DURATION = GameSettings::instance().AUTO_FR_DURATION0;
    bool TOLERATE_SYSTEM_UPDATE_MENU_SLOW = ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW;
    Milliseconds FULL_LOBBY_TIMER = GameSettings::instance().FULL_LOBBY_TIMER0;

    context.wait_for_all_requests();
    WallClock start = current_time();
    WallClock deadline_start_time = start + lobby_wait_delay;
    WallClock deadline_lobby_limit = start + FULL_LOBBY_TIMER;
    RaidLobbyReader inference(console.logger(), console.overlay());
    RaidLobbyState state;

    if (HOST_ONLINE && accept_FR_slot > 0){
        accept_FRs(
            console, context,
            accept_FR_slot - 1, true,
            GAME_TO_HOME_DELAY_SAFE,
            AUTO_FR_DURATION,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
        context.wait_for_all_requests();
        WallDuration time_elapsed = current_time() - start;
        WallDuration delay = time_elapsed;

        while (true){
            state = inference.read(console.video().snapshot());
            if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
                return state;
            }
            time_elapsed = current_time() - start;
            if (time_elapsed + delay >= lobby_wait_delay){
                break;
            }
            accept_FRs(
                console, context,
                accept_FR_slot - 1, false,
                GAME_TO_HOME_DELAY_SAFE,
                AUTO_FR_DURATION,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
            context.wait_for_all_requests();
        }
    }

    while (true){
        state = inference.read(console.video().snapshot());
        if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
            return state;
        }
        if (current_time() > deadline_start_time){
            break;
        }
        context.wait_for(std::chrono::milliseconds(1000));
    }

//    context.wait_for_all_requests();

    while (true){
        if (!state.valid || state.raiders_are_ready()){
            return state;
        }
        if (current_time() > deadline_lobby_limit){
            return state;
        }
        context.wait_for(std::chrono::milliseconds(1000));
        state = inference.read(console.video().snapshot());
    }
}



}
}
}
#endif

