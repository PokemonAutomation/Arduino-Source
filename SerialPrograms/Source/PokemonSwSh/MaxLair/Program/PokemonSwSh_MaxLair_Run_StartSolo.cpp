/*  Max Lair Run Start (Solo)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/VisualInferenceRoutines.h"
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
    ProgramEnvironment& env, ConsoleHandle& console,
    const QImage& entrance,
    std::chrono::system_clock::time_point time_limit
){
    LobbyDoneConnecting done_connecting_detector;
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        env, console,
        time_limit,
        {
            &done_connecting_detector,
            &entrance_detector,
            &false_start_detector,
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
    ProgramEnvironment& env, ConsoleHandle& console,
    const QImage& entrance,
    size_t min_players,
    size_t start_players,
    std::chrono::system_clock::time_point time_limit
){
    LobbyAllReadyDetector ready_detector(start_players);
    EntranceDetector entrance_detector(entrance);
    PokemonSelectMenuDetector false_start_detector(false);

    int result = wait_until(
        env, console,
        time_limit,
        {
            &ready_detector,
            &entrance_detector,
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
    ProgramEnvironment& env, ConsoleHandle& console,
    size_t consoles,
    const QImage& entrance
){
    LobbyMinReadyDetector ready_detector(consoles, true);
    if (ready_detector.ready_players(console.video().snapshot()) < consoles){
        console.log("Number of players less than expected. Did someone join the wrong lobby?", COLOR_RED);
        return false;
    }

    //  Press A until you're not in the lobby anymore.
    LobbyDetector lobby_detector(true);
    int result = run_until(
        env, console,
        [](const BotBaseContext& context){
            for (size_t c = 0; c < 180; c++){
                pbf_press_button(context, BUTTON_A, 10, 115);
                context.botbase().wait_for_all_requests();
            }
        },
        { &lobby_detector },
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
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    QImage& entrance, size_t boss_slot,
    ReadableQuantity999& ore
){
    env.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Enter lobby.
    entrance = enter_lobby(env, console, boss_slot, false, ore);
    if (entrance.isNull()){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(console);

    //  Start raid.
    pbf_press_dpad(console, DPAD_DOWN, 10, 50);
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    return true;
}

bool start_raid_host_solo(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    QImage& entrance, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ReadableQuantity999& ore
){
    env.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Start delay.
    env.wait_for(std::chrono::milliseconds(settings.START_DELAY * 1000 / TICKS_PER_SECOND));

    //  Enter lobby.
    entrance = enter_lobby(
        env, console, boss_slot,
        (HostingMode)(size_t)settings.MODE == HostingMode::HOST_ONLINE,
        ore
    );
    if (entrance.isNull()){
        return false;
    }

    //  Read boss.
    state.boss = read_boss_sprite(console);

    //  Enter code.
    uint8_t code[8];
    bool has_code = settings.RAID_CODE.get_code(code);
    if (has_code){
        pbf_press_button(console, BUTTON_PLUS, 10, TICKS_PER_SECOND);
        enter_digits(console, 8, code);
        pbf_wait(console, 2 * TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    }

    send_raid_notification(
        env.program_info(),
        console,
        settings.NOTIFICATIONS,
        has_code, code,
        state.boss,
        path_stats, session_stats
    );

    //  Open lobby.
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    auto time_limit = std::chrono::system_clock::now() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    if (!wait_for_a_player(env, console, entrance, time_limit)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return start_raid_self_solo(env, console, state_tracker, entrance, boss_slot, ore);
    }

    //  Ready up.
    env.wait_for(std::chrono::seconds(1));
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    //  Wait
    if (!wait_for_lobby_ready(env, console, entrance, 1, 4, time_limit)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    //  Start
    console.botbase().wait_for_all_requests();
    if (!start_adventure(env, console, 1, entrance)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    return true;
}



}
}
}
}
