/*  Auto-Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_DigitEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_AutoHostStats.h"
#include "PokemonSwSh_LobbyWait.h"
#include "PokemonSwSh_AutoHost.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool host_online,
    uint16_t connect_to_internet_delay
){
    if (!host_online){
        return true;
    }
    if (console.video().snapshot().isNull()){
        connect_to_internet(console, GameSettings::instance().OPEN_YCOMM_DELAY, connect_to_internet_delay);
        return true;
    }
    if (connect_to_internet_with_inference(
        env, console,
        std::chrono::seconds(5), connect_to_internet_delay
    )){
        return true;
    }
    return false;
}

void send_raid_notification(
    const QString& program_name,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const QImage& screenshot,
    const DenMonReadResults& results,
    const StatsTracker& stats_tracker
){
    if (!settings.enabled()){
        return;
    }

    std::vector<std::pair<QString, QString>> embeds;

    QString description = settings.DESCRIPTION;
    if (!description.isEmpty()){
        embeds.emplace_back("Description", description);
    }

    QString slugs;

    if (!screenshot.isNull()){
        if (results.type == DenMonReadResults::NOT_DETECTED){
            slugs += "Unable to detect - Not in den lobby.";
        }else if (results.slugs.results.empty()){
            slugs += "Unable to detect.";
        }else if (results.slugs.results.size() == 1){
            slugs += QString::fromStdString(results.slugs.results.begin()->second);
        }else{
            slugs += "Ambiguous: ";
            size_t c = 0;
            for (const auto& item : results.slugs.results){
                if (c > 0){
                    slugs += ", ";
                }
                slugs += QString::fromStdString(item.second);
                c++;
                if (c >= 5){
                    break;
                }
            }
            if (c < results.slugs.results.size()){
                slugs += ", (";
                slugs += QString::number(results.slugs.results.size() - c);
                slugs += " more...)";
            }
        }
        embeds.emplace_back("Current " + STRING_POKEMON, slugs);
    }

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

    if (!screenshot.isNull()){
        embeds.emplace_back("Session Stats", QString::fromStdString(stats_tracker.to_str()));
    }

    send_program_notification(
        console, settings.NOTIFICATION,
        QColor(),
        program_name,
        "Raid Notification",
        embeds,
        screenshot, false
    );

}


void run_autohost(
    ProgramEnvironment& env, ConsoleHandle& console,
    const QString& program_name,
    Catchability catchability, uint8_t skips,
    const RandomCodeOption* raid_code, uint16_t lobby_wait_delay,
    bool host_online, uint8_t accept_FR_slot,
    uint8_t move_slot, bool dynamax, uint8_t troll_hosting,
    AutoHostNotificationOption& notifications,
    uint16_t connect_to_internet_delay,
    uint16_t enter_online_den_delay,
    uint16_t open_online_den_lobby_delay,
    uint16_t raid_start_to_exit_delay,
    uint16_t delay_to_select_move
){
    AutoHostStats& stats = env.stats<AutoHostStats>();

    roll_den(
        console,
        enter_online_den_delay,
        open_online_den_lobby_delay,
        skips,
        catchability
    );
    console.botbase().wait_for_all_requests();

    if (!connect_to_internet(
        env, console,
        host_online,
        connect_to_internet_delay
    )){
        stats.add_timeout();
        env.log("Timed out waiting for internet connection. Skipping raid.", Qt::red);
        return;
    }

    {
        DenMonReader reader(console, console);
        enter_den(console, enter_online_den_delay, skips != 0, host_online);
        console.botbase().wait_for_all_requests();

        //  Make sure we're actually in a den.
        QImage screen = console.video().snapshot();
        DenMonReadResults results;
        if (!screen.isNull()){
            results = reader.read(screen);
            switch (results.type){
            case DenMonReadResults::RED_BEAM:
            case DenMonReadResults::PURPLE_BEAM:
                break;
            default:
                console.log("Failed to detect den lobby. Skipping raid.", Qt::red);
                return;
            }
        }

        uint8_t code[8];
        bool has_code = raid_code && raid_code->get_code(code);
        if (has_code){
            char str[8];
            for (size_t c = 0; c < 8; c++){
                str[c] = code[c] + '0';
            }
            env.log("Next Raid Code: " + std::string(str, sizeof(str)));
            pbf_press_button(console, BUTTON_PLUS, 5, 145);
            enter_digits(console, 8, code);
            pbf_wait(console, 180);
            pbf_press_button(console, BUTTON_A, 5, 95);
        }
        console.botbase().wait_for_all_requests();

        screen = console.video().snapshot();
        send_raid_notification(
            program_name,
            console,
            notifications,
            has_code, code,
            screen, results, stats
        );
    }

    enter_lobby(console, open_online_den_lobby_delay, host_online, catchability);

    //  Accept friend requests while we wait.
    RaidLobbyState raid_state = raid_lobby_wait(
        console,
        host_online,
        accept_FR_slot,
        lobby_wait_delay
    );

    //  Start Raid
    pbf_press_dpad(console, DPAD_UP, 5, 45);

    //  Mash A until it's time to close the game.
    {
        console.botbase().wait_for_all_requests();
        uint32_t start = system_clock(console);
        pbf_mash_button(console, BUTTON_A, 3 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        BlackScreenOverDetector black_screen;
        uint32_t now = start;
        while (true){
            if (black_screen.black_is_over(console.video().snapshot())){
                env.log("Raid has Started!", "blue");
                stats.add_raid(raid_state.raiders());
                break;
            }
            if (now - start >= raid_start_to_exit_delay){
                stats.add_timeout();
                break;
            }
            pbf_mash_button(console, BUTTON_A, TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();
            now = system_clock(console);
        }
    }

    //  Select a move.
    if (move_slot > 0){
        pbf_wait(console, delay_to_select_move);
        pbf_press_button(console, BUTTON_A, 20, 80);
        if (dynamax){
            pbf_press_dpad(console, DPAD_LEFT, 20, 30);
            pbf_press_button(console, BUTTON_A, 20, 60);
        }
        for (uint8_t c = 1; c < move_slot; c++){
            pbf_press_dpad(console, DPAD_DOWN, 20, 30);
        }
        pbf_press_button(console, BUTTON_A, 20, 80);

        // Disable the troll hosting option if the dynamax is set to TRUE.
        if (!dynamax && troll_hosting > 0){
            pbf_press_dpad(console, DPAD_DOWN, 20, 80);
            for (uint8_t c = 0; c < troll_hosting; c++){
                pbf_press_dpad(console, DPAD_RIGHT, 20, 80);
            }
        }

        pbf_press_button(console, BUTTON_A, 20, 980);
    }
}




}
}
}
