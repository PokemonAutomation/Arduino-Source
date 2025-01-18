/*  Max Lair Run Start (Solo)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh_MaxLair_Run_EnterLobby.h"
#include "PokemonSwSh_MaxLair_Run_StartSolo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




bool wait_for_a_player(
    ConsoleHandle& console, ControllerContext& context,
    const ImageViewRGB32& entrance,
    WallClock time_limit
){
    LobbyDoneConnecting done_connecting_detector;
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        console, context,
        time_limit,
        {
            {done_connecting_detector},
            {entrance_detector},
            {false_start_detector},
        },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("Connected to lobby.");
        break;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", COLOR_RED);
        return false;
    case 2:
        console.log("Detected false start. Did you join the wrong lobby?", COLOR_RED);
        return false;
    default:
        console.log("Timed out connecting to lobby.", COLOR_RED);
        return false;
    }
    return true;
}

bool wait_for_lobby_ready(
    ConsoleHandle& console, ControllerContext& context,
    const ImageViewRGB32& entrance,
    size_t min_players,
    size_t start_players,
    WallClock time_limit
){
    LobbyAllReadyDetector ready_detector(start_players);
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        console, context,
        time_limit,
        {
            {ready_detector},
            {entrance_detector},
        },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("Detected lobby ready...");
        return true;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", COLOR_RED);
        return false;
    case 2:
        console.log("Detected false start. Did you join the wrong lobby?", COLOR_RED);
        return false;
    }
    return true;
}
bool start_adventure(
    ConsoleHandle& console, ControllerContext& context,
    size_t consoles
){
    LobbyMinReadyDetector ready_detector(consoles, true);
    if (ready_detector.ready_players(console.video().snapshot()) < consoles){
        console.log("Number of players less than expected. Did someone join the wrong lobby?", COLOR_RED);
        return false;
    }

    //  Press A until you're not in the lobby anymore.
    LobbyDetector lobby_detector(true);
    int result = run_until<ControllerContext>(
        console, context,
        [](ControllerContext& context){
            for (size_t c = 0; c < 180; c++){
                pbf_press_button(context, BUTTON_A, 10, 115);
                context.wait_for_all_requests();
            }
        },
        {{lobby_detector}},
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        return true;
    default:
        console.log("Timed out starting raid.", COLOR_RED);
        return false;
    }
}




bool start_raid_self_solo(
    ConsoleHandle& console, ControllerContext& context,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32>& entrance, size_t boss_slot,
    ReadableQuantity999& ore
){
    console.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Enter lobby.
    entrance = enter_lobby(console, context, boss_slot, false, ore);
    if (!*entrance){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(console);

    //  Start raid.
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    return true;
}

bool start_raid_host_solo(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32>& entrance, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ReadableQuantity999& ore
){
    console.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Start delay.
    context.wait_for(std::chrono::milliseconds(settings.START_DELAY * 1000 / TICKS_PER_SECOND));

    //  Enter lobby.
    entrance = enter_lobby(
        console, context, boss_slot,
        settings.MODE == HostingMode::HOST_ONLINE,
        ore
    );
    if (!*entrance){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(console);

    //  Enter code.
    uint8_t code[8];
    bool has_code = settings.RAID_CODE.get_code(code);
    if (has_code){
        pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);
        enter_digits(context, 8, code);
        pbf_wait(context, 2 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }

    send_raid_notification(
        env,
        console,
        settings.NOTIFICATIONS,
        has_code, code,
        state.boss,
        path_stats, session_stats
    );

    //  Open lobby.
    pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    auto time_limit = current_time() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    if (!wait_for_a_player(console, context, *entrance, time_limit)){
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        return start_raid_self_solo(console, context, state_tracker, entrance, boss_slot, ore);
    }

    //  Ready up.
    context.wait_for(std::chrono::seconds(1));
    pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    //  Wait
    if (!wait_for_lobby_ready(console, context, *entrance, 1, 4, time_limit)){
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    //  Start
    context.wait_for_all_requests();
    if (!start_adventure(console, context, 1)){
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    return true;
}



}
}
}
}
