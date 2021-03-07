/*  Lobby Wait
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LobbyWait_H
#define PokemonAutomation_PokemonSwSh_LobbyWait_H

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShAutoHosts.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidLobbyReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

//#include <iostream>
//using std::cout;
//using std::endl;

static void raid_lobby_wait(
    ConsoleHandle& console, Logger& logger,
    bool HOST_ONLINE,
    uint8_t accept_FR_slot,
    uint16_t lobby_wait_delay
){
#if 0
    if (!settings.developer_mode){
        if (HOST_ONLINE && accept_FR_slot > 0){
            accept_FRs_while_waiting(
                accept_FR_slot - 1,
                lobby_wait_delay,
                GAME_TO_HOME_DELAY_SAFE,
                AUTO_FR_DURATION,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
        }else{
            pbf_wait(lobby_wait_delay);
        }
        return;
    }
#endif

    console.botbase().wait_for_all_requests();
    uint32_t start = system_clock();
    RaidLobbyReader inference(console, logger);
    RaidLobbyState state;

    if (HOST_ONLINE && accept_FR_slot > 0){
        accept_FRs(
            accept_FR_slot - 1, true,
            GAME_TO_HOME_DELAY_SAFE,
            AUTO_FR_DURATION,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
        console.botbase().wait_for_all_requests();
        uint32_t time_elapsed = system_clock() - start;
        uint32_t delay = time_elapsed;

        while (true){
            state = inference.read();
            if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
                return;
            }
            time_elapsed = system_clock() - start;
            if (time_elapsed + delay >= lobby_wait_delay){
                break;
            }
            accept_FRs(
                accept_FR_slot - 1, false,
                GAME_TO_HOME_DELAY_SAFE,
                AUTO_FR_DURATION,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
            console.botbase().wait_for_all_requests();
        }
    }

    while (true){
        state = inference.read();
        if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
            return;
        }
        uint32_t time_elapsed = system_clock() - start;
        if (time_elapsed >= lobby_wait_delay){
            break;
        }
        pbf_wait(std::min(lobby_wait_delay - time_elapsed, (uint32_t)TICKS_PER_SECOND));
        console.botbase().wait_for_all_requests();
    }

//    console.botbase().wait_for_all_requests();

    while (true){
        if (!state.valid || state.raiders_are_ready()){
            return;
        }
        uint32_t time_elapsed = system_clock() - start;
        if (time_elapsed > FULL_LOBBY_TIMER){
            return;
        }
        pbf_wait(std::min(FULL_LOBBY_TIMER - time_elapsed, (uint32_t)TICKS_PER_SECOND));
        console.botbase().wait_for_all_requests();
        state = inference.read();
    }
}



}
}
}
#endif

