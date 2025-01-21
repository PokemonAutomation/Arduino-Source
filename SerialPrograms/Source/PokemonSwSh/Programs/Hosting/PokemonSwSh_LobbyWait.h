/*  Lobby Wait
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LobbyWait_H
#define PokemonAutomation_PokemonSwSh_LobbyWait_H

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


static RaidLobbyState raid_lobby_wait(
    VideoStream& stream, SwitchControllerContext& context,
    bool HOST_ONLINE,
    uint8_t accept_FR_slot,
    uint16_t lobby_wait_delay
){
    uint16_t GAME_TO_HOME_DELAY_SAFE = GameSettings::instance().GAME_TO_HOME_DELAY_SAFE;
    uint16_t AUTO_FR_DURATION = GameSettings::instance().AUTO_FR_DURATION;
    bool TOLERATE_SYSTEM_UPDATE_MENU_SLOW = ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW;
    uint16_t FULL_LOBBY_TIMER = GameSettings::instance().FULL_LOBBY_TIMER;

    std::chrono::milliseconds lobby_wait_delay0((uint64_t)lobby_wait_delay * 1000 / TICKS_PER_SECOND);
    std::chrono::milliseconds FULL_LOBBY_TIMER0((uint64_t)FULL_LOBBY_TIMER * 1000 / TICKS_PER_SECOND);

    context.wait_for_all_requests();
    WallClock start = current_time();
    WallClock deadline_start_time = start + lobby_wait_delay0;
    WallClock deadline_lobby_limit = start + FULL_LOBBY_TIMER0;
//    uint32_t start = system_clock(context);
    RaidLobbyReader inference(stream.logger(), stream.overlay());
    RaidLobbyState state;

    if (HOST_ONLINE && accept_FR_slot > 0){
        accept_FRs(
            stream, context,
            accept_FR_slot - 1, true,
            GAME_TO_HOME_DELAY_SAFE,
            AUTO_FR_DURATION,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
        context.wait_for_all_requests();
        WallDuration time_elapsed = current_time() - start;
//        uint32_t time_elapsed = system_clock(context) - start;
        WallDuration delay = time_elapsed;
//        uint32_t delay = time_elapsed;

        while (true){
            state = inference.read(stream.video().snapshot());
            if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
                return state;
            }
            time_elapsed = current_time() - start;
            if (time_elapsed + delay >= lobby_wait_delay0){
                break;
            }
//            time_elapsed = system_clock(context) - start;
//            if (time_elapsed + delay >= lobby_wait_delay){
//                break;
//            }
            accept_FRs(
                stream, context,
                accept_FR_slot - 1, false,
                GAME_TO_HOME_DELAY_SAFE,
                AUTO_FR_DURATION,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
            context.wait_for_all_requests();
        }
    }

    while (true){
        state = inference.read(stream.video().snapshot());
        if (state.valid && state.raid_is_full() && state.raiders_are_ready()){
            return state;
        }
#if 1
        if (current_time() > deadline_lobby_limit){
            break;
        }
        context.wait_for(std::chrono::milliseconds(1000));
//        WallDuration time_elapsed = current_time() - start;
//        if (time_elapsed >= lobby_wait_delay0){
//            break;
//        }
//        context.wait_for(std::chrono::milliseconds(1000));
//        context.wait_for_all_requests();
//        context.wait_for(
//            std::min(
//                lobby_wait_delay0 - std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed),
//                std::chrono::milliseconds(1000)
//            )
//        );
#else
        uint32_t time_elapsed = system_clock(context) - start;
        if (time_elapsed >= lobby_wait_delay){
            break;
        }
        pbf_wait(
            context,
            (uint16_t)std::min(
                lobby_wait_delay - time_elapsed,
                (uint32_t)TICKS_PER_SECOND
            )
        );
        context.wait_for_all_requests();
#endif
    }

//    context.wait_for_all_requests();

    while (true){
        if (!state.valid || state.raiders_are_ready()){
            return state;
        }
        if (current_time() > deadline_start_time){
            return state;
        }
        context.wait_for(std::chrono::milliseconds(1000));
#if 0
        uint32_t time_elapsed = system_clock(context) - start;
        if (time_elapsed > FULL_LOBBY_TIMER){
            return state;
        }
        pbf_wait(
            context,
            (uint16_t)std::min(
                FULL_LOBBY_TIMER - time_elapsed,
                (uint32_t)TICKS_PER_SECOND
            )
        );
        context.wait_for_all_requests();
#endif
        state = inference.read(stream.video().snapshot());
    }
}



}
}
}
#endif

