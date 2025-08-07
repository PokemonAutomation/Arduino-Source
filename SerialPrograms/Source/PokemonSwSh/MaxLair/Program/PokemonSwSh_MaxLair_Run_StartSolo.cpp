/*  Max Lair Run Start (Solo)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
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
    VideoStream& stream, ProControllerContext& context,
    const ImageViewRGB32& entrance,
    WallClock time_limit
){
    LobbyDoneConnecting done_connecting_detector;
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        stream, context,
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
        stream.log("Connected to lobby.");
        break;
    case 1:
        stream.log("Detected entrance... Did you get disconnected?", COLOR_RED);
        return false;
    case 2:
        stream.log("Detected false start. Did you join the wrong lobby?", COLOR_RED);
        return false;
    default:
        stream.log("Timed out connecting to lobby.", COLOR_RED);
        return false;
    }
    return true;
}

bool wait_for_lobby_ready(
    VideoStream& stream, ProControllerContext& context,
    const ImageViewRGB32& entrance,
    size_t min_players,
    size_t start_players,
    WallClock time_limit
){
    LobbyAllReadyDetector ready_detector(start_players);
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        stream, context,
        time_limit,
        {
            {ready_detector},
            {entrance_detector},
        },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        stream.log("Detected lobby ready...");
        return true;
    case 1:
        stream.log("Detected entrance... Did you get disconnected?", COLOR_RED);
        return false;
    case 2:
        stream.log("Detected false start. Did you join the wrong lobby?", COLOR_RED);
        return false;
    }
    return true;
}
bool start_adventure(
    VideoStream& stream, ProControllerContext& context,
    size_t consoles
){
    LobbyMinReadyDetector ready_detector(consoles, true);
    if (ready_detector.ready_players(stream.video().snapshot()) < consoles){
        stream.log("Number of players less than expected. Did someone join the wrong lobby?", COLOR_RED);
        return false;
    }

    //  Press A until you're not in the lobby anymore.
    LobbyDetector lobby_detector(true);
    int result = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
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
        stream.log("Timed out starting raid.", COLOR_RED);
        return false;
    }
}




bool start_raid_self_solo(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32>& entrance, size_t boss_slot,
    ReadableQuantity999& ore
){
    stream.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Enter lobby.
    entrance = enter_lobby(info, stream, context, boss_slot, false, ore);
    if (!*entrance){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(stream);

    //  Start raid.
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    return true;
}

bool start_raid_host_solo(
    ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
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
    context.wait_for(settings.START_DELAY0);

    //  Enter lobby.
    entrance = enter_lobby(
        env.program_info(), console, context,
        boss_slot,
        settings.MODE == HostingMode::HOST_ONLINE,
        ore
    );
    if (!*entrance){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(console);

    //  Enter code.
    std::string code = settings.RAID_CODE.get_code();
    if (!code.empty()){
        pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);
        FastCodeEntry::numberpad_enter_code(console, context, code, true);
        pbf_wait(context, 2 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }

    send_raid_notification(
        env,
        console,
        settings.NOTIFICATIONS,
        code,
        state.boss,
        path_stats, session_stats
    );

    //  Open lobby.
    pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    auto time_limit = current_time() + settings.LOBBY_WAIT_DELAY0.get();

    if (!wait_for_a_player(console, context, *entrance, time_limit)){
        pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
        return start_raid_self_solo(
            env.program_info(), console, context,
            state_tracker,
            entrance,
            boss_slot,
            ore
        );
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
