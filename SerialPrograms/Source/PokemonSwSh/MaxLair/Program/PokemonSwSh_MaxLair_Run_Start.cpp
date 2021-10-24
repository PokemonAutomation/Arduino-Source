/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_DigitEntry.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh_MaxLair_Run_Start.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


QImage enter_lobby(ConsoleHandle& console, size_t boss_slot){
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    QImage entrance = console.video().snapshot();

    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    pbf_press_button(console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);

    if (boss_slot > 0){
        for (size_t c = 1; c < boss_slot; c++){
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
        }
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    }else{
        pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
    }
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    pbf_press_button(console, BUTTON_A, 10, 5 * TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    return entrance;
}
bool wait_for_a_player(
    ProgramEnvironment& env, ConsoleHandle& console,
    const QImage& entrance,
    std::chrono::system_clock::time_point time_limit
){
    LobbyDoneConnecting done_connecting_detector;
    EntranceDetector entrance_detector(entrance);
    int result = wait_until(
        env, console,
        time_limit,
        {
            &done_connecting_detector,
            &entrance_detector,
        },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("Connected to lobby.");
        break;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_a_player", console.video().snapshot());
        return false;
    default:
        console.log("Timed out connecting to lobby.", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_a_player", console.video().snapshot());
        return false;
    }
    return true;
}
bool wait_for_all_join(
    ProgramEnvironment& env, ConsoleHandle& console,
    const QImage& entrance,
    size_t start_players
){
    LobbyJoinedDetector joined_detector(start_players);
    EntranceDetector entrance_detector(entrance);
    int result = wait_until(
        env, console,
        std::chrono::seconds(5),
        { &joined_detector },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("All consoles joined!");
        return true;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_all_join", console.video().snapshot());
        return false;
    default:
        console.log("Timed out waiting for everyone to join.", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_all_join", console.video().snapshot());
        return false;
    }
}
bool wait_for_lobby_ready(
    ProgramEnvironment& env, ConsoleHandle& console,
    const QImage& entrance,
    size_t start_players,
    std::chrono::system_clock::time_point time_limit
){
    LobbyAllReadyDetector ready_detector(start_players, time_limit);
    EntranceDetector entrance_detector(entrance);
    int result = wait_until(
        env, console,
        std::chrono::seconds(300),
        { &ready_detector },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("Detected lobby ready...");
        return true;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_lobby_ready", console.video().snapshot());
        return false;
    default:
        console.log("Timed out...", Qt::red);
//        dump_image(console, MODULE_NAME, "wait_for_lobby_ready", console.video().snapshot());
        return false;
    }
}
bool start_raid(ProgramEnvironment& env, ConsoleHandle& console, const QImage& entrance){
    SelectionArrowFinder arrow_detector(console, ImageFloatBox(0.550, 0.600, 0.200, 0.200));
    NonLobbyDetector menu_detector;
    EntranceDetector entrance_detector(entrance);
    std::chrono::system_clock::time_point stop = std::chrono::system_clock::now() + std::chrono::seconds(180);
    while (true){
        int result = wait_until(
            env, console,
            stop,
            {
                &arrow_detector,
                &menu_detector,
                &entrance_detector,
            },
            INFERENCE_RATE
        );
        switch (result){
        case 0:
            pbf_press_button(console, BUTTON_A, 10, 115);
            console.botbase().wait_for_all_requests();
            break;
        case 1:
            return true;
        case 2:
            return false;
        default:
            return false;
        }
    }
}

class AllJoinedTracker{
public:
    AllJoinedTracker(
        ProgramEnvironment& env, size_t consoles,
        std::chrono::system_clock::time_point time_limit
    )
        : m_env(env)
        , m_time_limit(time_limit)
        , m_consoles(consoles)
        , m_counter(0)
    {}

    bool report_joined(){
        std::unique_lock<std::mutex> lg(m_env.lock());
        m_counter++;
        if (m_counter >= m_consoles){
            m_env.cv().notify_all();
            return true;
        }
        while (true){
            m_env.cv().wait_until(lg, m_time_limit);
            m_env.check_stopping();
            if (m_counter >= m_consoles){
                return true;
            }
            if (std::chrono::system_clock::now() > m_time_limit){
                return false;
            }
        }
    }

private:
    ProgramEnvironment& m_env;
    std::chrono::system_clock::time_point m_time_limit;
    size_t m_consoles;
    size_t m_counter;
};



void send_raid_notification(
    const QString& program_name,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const std::string& slug,
    const StatsTracker& stats_tracker
){
    if (!settings.enabled()){
        return;
    }

    QImage screen = console.video().snapshot();

    std::vector<std::pair<QString, QString>> embeds;

    QString description = settings.DESCRIPTION;
    if (!description.isEmpty()){
        embeds.emplace_back("Description", description);
    }

    QString slugs;
    if (slug.empty()){
        slugs += "Unable to detect.";
    }else{
        slugs += get_pokemon_name(get_maxlair_slugs(slug).name_slug).display_name();
    }
    embeds.emplace_back("Current " + STRING_POKEMON, slugs);

    {
        std::string code_str;
        if (has_code){
            size_t c = 0;
            for (; c < 4; c++){
                code_str += code[c] + '0';
            }
            code_str += " ";
            for (; c < 8; c++){
                code_str += code[c] + '0';
            }
        }else{
            code_str += "None";
        }
        embeds.emplace_back("Raid Code", QString::fromStdString(code_str));
    }

    embeds.emplace_back("Session Stats", QString::fromStdString(stats_tracker.to_str()));

    send_program_notification(
        console, settings.NOTIFICATION,
        QColor(),
        program_name,
        "Max Lair Notification",
        embeds,
        screen, false
    );

}






bool start_raid_self_solo(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    QImage& entrance, size_t boss_slot
){
    env.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Clear quit prompts.
    pbf_press_button(console, BUTTON_A, 10, 10);
    pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);

    //  Enter lobby.
    entrance = enter_lobby(console, boss_slot);

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
    const QString& program_name, const StatsTracker& stats_tracker
){
    env.log("Entering lobby...");

    GlobalState& state = state_tracker[0];

    //  Clear quit prompts.
    pbf_press_button(console, BUTTON_A, 10, 10);
    pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);

    if ((HostingMode)(size_t)settings.MODE == HostingMode::HOST_ONLINE){
//        connect_to_internet(console, OPEN_YCOMM_DELAY, settings.CONNECT_TO_INTERNET_DELAY);
        connect_to_internet_with_inference(env, console);
    }

    //  Enter lobby.
    entrance = enter_lobby(console, boss_slot);

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
        program_name,
        console,
        settings.NOTIFICATIONS,
        has_code, code,
        state.boss, stats_tracker
    );

    //  Open lobby.
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    auto time_limit = std::chrono::system_clock::now() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    if (!wait_for_a_player(env, console, entrance, time_limit)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return start_raid_self_solo(env, console, state_tracker, entrance, boss_slot);
    }

    //  Ready up.
    env.wait_for(std::chrono::seconds(1));
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    //  Wait
    if (!wait_for_lobby_ready(env, console, entrance, 4, time_limit)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    //  Start
    console.botbase().wait_for_all_requests();
    if (!start_raid(env, console, entrance)){
        pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
        return false;
    }

    return true;
}

bool start_raid_local(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    QImage entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    const HostingSettings& settings
){
    if (env.consoles.size() == 1){
        return start_raid_self_solo(env, host, state_tracker, entrance[0], boss_slot);
    }

    env.log("Entering lobby...");

    uint8_t code[8];
    bool raid_code = settings.RAID_CODE.get_code(code);

    env.run_in_parallel([&](ConsoleHandle& console){
        size_t index = console.index();
        GlobalState& state = state_tracker[index];

        bool is_host = index == host.index();

        //  Clear quit prompts.
        pbf_press_button(console, BUTTON_A, 10, 10);
        pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);

        //  Enter lobby.
        entrance[index] = enter_lobby(console, is_host ? boss_slot : 0);

        //  Read boss.
        if (is_host){
            state.boss = read_boss_sprite(console);
        }

        //  Enter code.
        if (raid_code && env.consoles.size() > 1){
            pbf_press_button(console, BUTTON_PLUS, 10, TICKS_PER_SECOND);
            enter_digits(console, 8, code);
            pbf_wait(console, 2 * TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        }
    });

    //  Open lobby.
    env.run_in_parallel([&](ConsoleHandle& console){
        //  Delay to prevent the Switches from forming separate lobbies.
        if (env.consoles.size() > 2 && console.index() != host.index()){
            pbf_wait(console, 3 * TICKS_PER_SECOND);
        }
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    });

    auto time_limit = std::chrono::system_clock::now() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    AllJoinedTracker joined_tracker(env, env.consoles.size(), time_limit);


    std::atomic<size_t> errors(0);

    //  Wait for all Switches to join.
    env.run_in_parallel([&](ConsoleHandle& console){
        size_t index = console.index();

        //  Wait for a player to show up. This lets you ready up.
        if (!wait_for_a_player(env, console, entrance[index], time_limit)){
            errors.fetch_add(1);
            return;
        }

        //  Wait for all consoles to join.
        if (!joined_tracker.report_joined()){
            console.log("Not everyone was able to join.", Qt::red);
            errors.fetch_add(1);
            return;
        }

        if (!wait_for_all_join(env, console, entrance[index], env.consoles.size())){
            console.log("Switches joined into different raids.", Qt::red);
            errors.fetch_add(1);
            return;
        }
    });
    if (errors.load(std::memory_order_acquire) != 0){
        env.run_in_parallel([&](ConsoleHandle& console){
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_mash_button(console, BUTTON_B, 8 * TICKS_PER_SECOND);
        });
        return false;
    }

    //  Ready up and wait for lobby to be ready.
    env.run_in_parallel([&](ConsoleHandle& console){
        //  Ready up.
        env.wait_for(std::chrono::seconds(1));
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        //  Wait
        size_t index = console.index();
        if (!wait_for_lobby_ready(env, console, entrance[index], env.consoles.size(), time_limit)){
            errors.fetch_add(1);
            pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }

        //  Start
        if (!start_raid(env, console, entrance[index])){
            errors.fetch_add(1);
            pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });

    return errors.load(std::memory_order_acquire) == 0;
}

bool start_raid_host(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    QImage entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    HostingSettings& settings,
    const QString& program_name, const StatsTracker& stats_tracker
){
    if (env.consoles.size() == 1){
        return start_raid_host_solo(
            env, host,
            state_tracker,
            entrance[0], boss_slot,
            settings,
            program_name, stats_tracker
        );
    }

    env.log("Entering lobby...");

    uint8_t code[8];
    bool has_code = settings.RAID_CODE.get_code(code);
    std::string boss;

    env.run_in_parallel([&](ConsoleHandle& console){
        size_t index = console.index();
        GlobalState& state = state_tracker[index];

        bool is_host = index == host.index();

        //  Clear quit prompts.
        pbf_press_button(console, BUTTON_A, 10, 10);
        pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);

        if ((HostingMode)(size_t)settings.MODE == HostingMode::HOST_ONLINE){
//            connect_to_internet(console, OPEN_YCOMM_DELAY, settings.CONNECT_TO_INTERNET_DELAY);
            connect_to_internet_with_inference(env, console);
        }

        //  Enter lobby.
        entrance[index] = enter_lobby(console, is_host ? boss_slot : 0);

        //  Read boss.
        if (is_host){
            boss = read_boss_sprite(console);
            state.boss = boss;
        }

        //  Enter Code
        if (has_code){
            pbf_press_button(console, BUTTON_PLUS, 10, TICKS_PER_SECOND);
            enter_digits(console, 8, code);
            pbf_wait(console, 2 * TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        }
    });

    //  Open lobby.
    env.run_in_parallel([&](ConsoleHandle& console){
        //  If you start the raids at the same time, they won't find each other.
        if (console.index() != host.index()){
            pbf_wait(console, 3 * TICKS_PER_SECOND);
        }
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    });

    auto time_limit = std::chrono::system_clock::now() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    AllJoinedTracker joined_tracker(env, env.consoles.size(), time_limit);

    std::atomic<size_t> errors(0);

    //  Wait for all Switches to join.
    env.run_in_parallel([&](ConsoleHandle& console){
        size_t index = console.index();

        //  Wait for a player to show up. This lets you ready up.
        if (!wait_for_a_player(env, console, entrance[index], time_limit)){
            errors.fetch_add(1);
            return;
        }

        //  Wait for all consoles to join.
        if (!joined_tracker.report_joined()){
            console.log("Not everyone was able to join.", Qt::red);
            errors.fetch_add(1);
            return;
        }

        if (!wait_for_all_join(env, console, entrance[index], env.consoles.size())){
            console.log("Switches joined into different raids.", Qt::red);
            errors.fetch_add(1);
            return;
        }
    });
    if (errors.load(std::memory_order_acquire) != 0){
        env.run_in_parallel([&](ConsoleHandle& console){
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_mash_button(console, BUTTON_B, 8 * TICKS_PER_SECOND);
        });
        return false;
    }

    send_raid_notification(
        program_name,
        host,
        settings.NOTIFICATIONS,
        has_code, code,
        boss, stats_tracker
    );

    //  Ready up and wait for lobby to be ready.
    env.run_in_parallel([&](ConsoleHandle& console){
        //  Ready up.
        env.wait_for(std::chrono::seconds(1));
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        //  Wait
        size_t index = console.index();
        if (!wait_for_lobby_ready(env, console, entrance[index], 4, time_limit)){
            errors.fetch_add(1);
            pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }

        //  Start
        if (!start_raid(env, console, entrance[index])){
            errors.fetch_add(1);
            pbf_mash_button(console, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });

    return errors.load(std::memory_order_acquire) == 0;
}



bool start_raid(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    QImage entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    HostingSettings& settings,
    const QString& program_name, const StatsTracker& stats_tracker
){
    switch ((HostingMode)(size_t)settings.MODE){
    case HostingMode::NOT_HOSTING:
        return start_raid_local(env, state_tracker, entrance, host, boss_slot, settings);
    case HostingMode::HOST_LOCALLY:
    case HostingMode::HOST_ONLINE:
        return start_raid_host(env, state_tracker, entrance, host, boss_slot, settings, program_name, stats_tracker);
    }
    PA_THROW_StringException("Invalid mode enum.");
}










}
}
}
}
