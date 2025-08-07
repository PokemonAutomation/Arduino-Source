/*  Auto-Hosting
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
#include "Pokemon/Pokemon_Strings.h"
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

using namespace Pokemon;


bool connect_to_internet(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    bool host_online,
    Milliseconds connect_to_internet_delay
){
    if (!host_online){
        return true;
    }
    if (!stream.video().snapshot()){
        connect_to_internet(context, GameSettings::instance().OPEN_YCOMM_DELAY0, connect_to_internet_delay);
        return true;
    }
    if (connect_to_internet_with_inference(
        info, stream, context,
        std::chrono::seconds(5), connect_to_internet_delay
    )){
        return true;
    }
    return false;
}

void send_raid_notification(
    ProgramEnvironment& env,
    VideoStream& stream,
    AutoHostNotificationOption& settings,
    const std::string& code,
    const ImageViewRGB32& screenshot,
    const DenMonReadResults& results,
    const StatsTracker& stats_tracker
){
    if (!settings.enabled()){
        return;
    }

    std::vector<std::pair<std::string, std::string>> embeds;

    std::string description = settings.DESCRIPTION;
    if (!description.empty()){
        embeds.emplace_back("Description:", description);
    }

    std::string slugs;

    if (results.type == DenMonReadResults::NOT_DETECTED){
        slugs += "Unable to detect - Not in den lobby.";
    }else if (results.slugs.results.empty()){
        slugs += "Unable to detect.";
    }else if (results.slugs.results.size() == 1){
        slugs += results.slugs.results.begin()->second;
    }else{
        slugs += "Ambiguous: ";
        size_t c = 0;
        for (const auto& item : results.slugs.results){
            if (c > 0){
                slugs += ", ";
            }
            slugs += item.second;
            c++;
            if (c >= 5){
                break;
            }
        }
        if (c < results.slugs.results.size()){
            slugs += ", (";
            slugs += std::to_string(results.slugs.results.size() - c);
            slugs += " more...)";
        }
    }
    embeds.emplace_back("Current " + STRING_POKEMON + ":", slugs);

    embeds.emplace_back("Raid Code:", code.empty() ? "None" : code);

    send_program_notification(
        env, settings.NOTIFICATION,
        Color(),
        "Max Raid Notification",
        embeds, "",
        screenshot, false
    );

}


void run_autohost(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    Catchability catchability, uint8_t skips,
    const RandomCodeOption* raid_code, Milliseconds lobby_wait_delay,
    bool host_online, uint8_t accept_FR_slot,
    uint8_t move_slot, bool dynamax, uint8_t troll_hosting,
    AutoHostNotificationOption& notifications,
    Milliseconds connect_to_internet_delay,
    Milliseconds enter_online_den_delay,
    Milliseconds open_online_den_lobby_delay,
    Milliseconds raid_start_to_exit_delay,
    Milliseconds delay_to_select_move
){
    AutoHostStats& stats = env.current_stats<AutoHostStats>();

    roll_den(
        console, context,
        enter_online_den_delay,
        open_online_den_lobby_delay,
        skips,
        catchability
    );
    context.wait_for_all_requests();

    if (!connect_to_internet(
        env.program_info(), console, context,
        host_online,
        connect_to_internet_delay
    )){
        stats.add_timeout();
        env.log("Timed out waiting for internet connection. Skipping raid.", COLOR_RED);
        return;
    }

    {
        DenMonReader reader(console.logger(), console.overlay());
        enter_den(context, enter_online_den_delay, skips != 0, host_online);
        context.wait_for_all_requests();

        //  Make sure we're actually in a den.
        VideoSnapshot screen = console.video().snapshot();
        DenMonReadResults results;
        if (screen){
            results = reader.read(screen);
            switch (results.type){
            case DenMonReadResults::RED_BEAM:
            case DenMonReadResults::PURPLE_BEAM:
                break;
            default:
                console.log("Failed to detect den lobby. Skipping raid.", COLOR_RED);
                return;
            }
        }

        std::string code;
        if (raid_code){
            code = raid_code->get_code();
        }
        if (!code.empty()){
            env.log("Next Raid Code: " + code);
            pbf_press_button(context, BUTTON_PLUS, 5, 145);
            FastCodeEntry::numberpad_enter_code(console, context, code, true);
            pbf_wait(context, 180);
            pbf_press_button(context, BUTTON_A, 5, 95);
        }
        context.wait_for_all_requests();

        screen = console.video().snapshot();
        send_raid_notification(
            env,
            console,
            notifications,
            code,
            screen, results, stats
        );
    }

    enter_lobby(context, open_online_den_lobby_delay, host_online, catchability);

    //  Accept friend requests while we wait.
    RaidLobbyState raid_state = raid_lobby_wait(
        console, context,
        host_online,
        accept_FR_slot,
        lobby_wait_delay
    );

    //  Start Raid
    pbf_press_dpad(context, DPAD_UP, 5, 45);

    //  Mash A until it's time to close the game.
    if (console.video().snapshot()){
        BlackScreenOverWatcher black_screen;
        int ret = run_until<ProControllerContext>(
            console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, raid_start_to_exit_delay);
            },
            {black_screen}
        );
        if (ret == 0){
            env.log("Raid has Started!", COLOR_BLUE);
            stats.add_raid(raid_state.raiders());
        }else{
            env.log("Timed out waiting for raid to start.", COLOR_RED);
            stats.add_timeout();
        }
    }else{
        pbf_mash_button(context, BUTTON_A, raid_start_to_exit_delay);
    }

    //  Select a move.
    if (move_slot > 0){
        pbf_wait(context, delay_to_select_move);
        pbf_press_button(context, BUTTON_A, 20, 80);
        if (dynamax){
            pbf_press_dpad(context, DPAD_LEFT, 20, 30);
            pbf_press_button(context, BUTTON_A, 20, 60);
        }
        for (uint8_t c = 1; c < move_slot; c++){
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
        }
        pbf_press_button(context, BUTTON_A, 20, 80);

        // Disable the troll hosting option if the dynamax is set to TRUE.
        if (!dynamax && troll_hosting > 0){
            pbf_press_dpad(context, DPAD_DOWN, 20, 80);
            for (uint8_t c = 0; c < troll_hosting; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 20, 80);
            }
        }

        pbf_press_button(context, BUTTON_A, 20, 980);
    }
}




}
}
}
