/*  Tera Exit Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
//#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokePortalDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRewardsReader.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSV_TeraRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



bool open_raid(ConsoleHandle& console, BotBaseContext& context){
    console.log("Opening raid...");
    while (true){
        TeraCardWatcher card_detector(COLOR_RED);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                //  Do 2 presses in quick succession in case one drops or is
                //  needed to connect the controller.
                pbf_press_button(context, BUTTON_A, 5, 5);
                pbf_press_button(context, BUTTON_A, 20, 355);
            },
            {
                card_detector,
                dialog,
            }
        );
        switch (ret){
        case 0:
            console.log("Tera raid found!", COLOR_BLUE);
            return true;
        case 1:
            console.log("Detect possible uncatchable dialog...", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            console.log("No Tera raid found.", COLOR_ORANGE);
            return false;
        }
    }
}
void close_raid(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Closing raid...");

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "CloseRaidFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCardWatcher card_detector(COLOR_RED);
        OverworldWatcher overworld(COLOR_CYAN);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {card_detector, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            console.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(info, console, "CloseRaidFailed",
                "close_raid(): No recognized state after 60 seconds.");
        }
    }
}



void open_hosting_lobby(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    HostingMode mode
){
    bool recovery_mode = false;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(2)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "OpenLobbyFailed",
                "Unable to open Tera lobby after 2 minutes."
            );
        }

        OverworldWatcher overworld(COLOR_RED);
        if (recovery_mode){
            context.wait_for_all_requests();
            int ret = run_until(
                console, context,
                [](BotBaseContext& context){
                    pbf_press_button(context, BUTTON_B, 20, 980);
                },
                {overworld}
            );
            if (ret < 0){
                continue;
            }
            console.log("Detected overworld. Recovery finished.");
            recovery_mode = true;
        }

        TeraCardWatcher card_detector(COLOR_YELLOW);
        TeraLobbyWatcher lobby(console.logger(), env.realtime_dispatcher(), COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                card_detector,
                {lobby, std::chrono::milliseconds(500)}
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            recovery_mode = false;
            if (!open_raid(console, context)){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "No Tera raid found.",
                    true
                );
            }
            continue;
        case 1:
            console.log("Detected Tera card.");
            if (mode != HostingMode::LOCAL){
                pbf_press_button(context, BUTTON_A, 20, 230);
                if (mode == HostingMode::ONLINE_EVERYONE){
                    pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                }
            }
            pbf_press_button(context, BUTTON_A, 20, 230);
            continue;
        case 2:
            console.log("Detected Tera lobby.");
            return;
        default:
            console.log("No state detected after 30 seconds. Backing out...", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 230);
            recovery_mode = true;
        }
    }
}





void enter_tera_search(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    bool connect_to_internet
){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "EnterTeraSearchFailed",
                "enter_tera_search(): Failed to enter Tera search."
            );
        }

        OverworldWatcher overworld(COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        PokePortalWatcher poke_portal(COLOR_GREEN);
        TeraRaidSearchWatcher raid_search(COLOR_CYAN);
        CodeEntryWatcher code_entry(COLOR_PURPLE);
        AdvanceDialogWatcher dialog(COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                poke_portal,
                raid_search,
                code_entry,
                dialog,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (connect_to_internet && !connected){
                connect_to_internet_from_menu(info, console, context);
                connected = true;
                continue;
            }
            if (main_menu.move_cursor(info, console, context, MenuSide::RIGHT, 3)){
                pbf_press_button(context, BUTTON_A, 20, 230);
            }
            continue;
        case 2:
            console.log("Detected Poke Portal.");
            if (poke_portal.move_cursor(info, console, context, 1)){
                pbf_press_button(context, BUTTON_A, 20, 230);
            }
            continue;
        case 3:
            console.log("Detected Tera Raid Search.");
            if (raid_search.move_cursor_to_search(info, console, context)){
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
            continue;
        case 4:
            console.log("Detected Code Entry.");
            return;
        case 5:
            console.log("Detected Dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            dump_image_and_throw_recoverable_exception(
                info, console, "EnterTeraSearchFailed",
                "enter_tera_search(): No recognized state after 30 seconds."
            );
        }
    }
}





void stop_if_enough_rare_items(
    ConsoleHandle& console, BotBaseContext& context,
    size_t stop_on_sparkly_items
){
    if (stop_on_sparkly_items == 0){
        return;
    }
    size_t sparkly_items = SparklyItemDetector::count_sparkly_items(console, context);
    console.log("Sparkly Items Detected: " + std::to_string(sparkly_items), COLOR_BLUE);
    if (sparkly_items >= stop_on_sparkly_items){
        throw ProgramFinishedException(console, "Found a raid with " + std::to_string(sparkly_items) + " rare items!", true);
    }
}


void exit_tera_win_without_catching(
    const ProgramInfo& info,
    ConsoleHandle& console, BotBaseContext& context,
    size_t stop_on_sparkly_items
){
    console.log("Exiting raid without catching...");

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "ExitTeraWinFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA,
            {0.8, 0.93, 0.2, 0.07},
            WhiteButtonWatcher::FinderType::PRESENT,
            std::chrono::seconds(1)
        );
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        OverworldWatcher overworld(COLOR_RED);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {catch_menu, next_button, dialog, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected catch prompt.");
            catch_menu.move_to_slot(console, context, 1);
//            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_mash_button(context, BUTTON_A, 30);
            pbf_mash_button(context, BUTTON_B, 125);
            continue;
        case 1:
            console.log("Detected possible (A) Next button.");
            stop_if_enough_rare_items(console, context, stop_on_sparkly_items);
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 3:
            console.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(
                info, console, "ExitTeraWinFailed",
                "exit_tera_win_without_catching(): No recognized state after 60 seconds."
            );
        }
    }
}


void exit_tera_win_by_catching(
    ProgramEnvironment& env,
    ConsoleHandle& console, BotBaseContext& context,
    Language language,
    const std::string& ball_slug,
    size_t stop_on_sparkly_items
){
    console.log("Exiting raid with catching...");

    VideoSnapshot screenshot;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "ExitTeraWinFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA,
            {0.8, 0.93, 0.2, 0.07},
            WhiteButtonWatcher::FinderType::PRESENT,
            std::chrono::seconds(1)
        );
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        PromptDialogWatcher add_to_party(COLOR_PURPLE, {0.500, 0.395, 0.400, 0.100});
        PromptDialogWatcher nickname(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});
        MainMenuWatcher main_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {
                catch_menu,
                next_button,
                advance,
                add_to_party,
                nickname,
                main_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            console.log("Detected catch prompt.");
            screenshot = console.video().snapshot();

            catch_menu.move_to_slot(console, context, 0);
            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(console, language);
            int quantity = move_to_ball(reader, console, context, ball_slug);
            if (quantity == 0){
                throw FatalProgramException(
                    ErrorReport::NO_ERROR_REPORT, console,
                    "Unable to find appropriate ball. Did you run out?",
                    true
                );
            }
            if (quantity < 0){
                console.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 1:
            console.log("Detected (A) Next button.");
            stop_if_enough_rare_items(console, context, stop_on_sparkly_items);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Detected add-to-party prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 4:
            console.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 5:
            console.log("Detected unexpected main menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 6:
            console.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "ExitTeraWinFailed",
                "exit_tera_win_by_catching(): No recognized state after 60 seconds."
            );
        }
    }
}


TeraResult exit_tera_win_by_catching(
    ProgramEnvironment& env,
    ConsoleHandle& console, BotBaseContext& context,
    Language language,
    const std::string& ball_slug,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, size_t stop_on_sparkly_items,
    std::atomic<uint64_t>* stat_shinies
){
    console.log("Exiting raid with catching...");

    TeraResult result = TeraResult::NO_DETECTION;
    VideoSnapshot screenshot;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "ExitTeraWinFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA,
            {0.8, 0.93, 0.2, 0.07},
            WhiteButtonWatcher::FinderType::PRESENT,
            std::chrono::seconds(1)
        );
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        PromptDialogWatcher add_to_party(COLOR_PURPLE, {0.500, 0.395, 0.400, 0.100});
        PromptDialogWatcher view_summary(COLOR_PURPLE, {0.500, 0.470, 0.400, 0.100});
        PromptDialogWatcher nickname(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});
        PokemonSummaryWatcher summary(COLOR_MAGENTA);
        MainMenuWatcher main_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {
                catch_menu,
                next_button,
                advance,
                add_to_party,
                view_summary,
                nickname,
                summary,
                main_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            console.log("Detected catch prompt.");
            screenshot = console.video().snapshot();

            catch_menu.move_to_slot(console, context, 0);
            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(console, language);
            int quantity = move_to_ball(reader, console, context, ball_slug);
            if (quantity == 0){
                throw FatalProgramException(
                    ErrorReport::NO_ERROR_REPORT, console,
                    "Unable to find appropriate ball. Did you run out?",
                    true
                );
            }
            if (quantity < 0){
                console.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 2:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Detected add-to-party prompt.");
            if (result == TeraResult::NO_DETECTION){
                pbf_press_dpad(context, DPAD_DOWN, 20, 60);
//                pbf_press_button(context, BUTTON_A, 20, 105);
            }else{
                pbf_press_button(context, BUTTON_B, 20, 105);
            }
            continue;
        case 4:
            console.log("Detected cursor over view summary.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 5:
            console.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            //  Next button detector is unreliable. Check if the summary is
            //  open. If so, fall-through to that.
            if (!summary.detect(console.video().snapshot())){
                console.log("Detected possible (A) Next button.");
                stop_if_enough_rare_items(console, context, stop_on_sparkly_items);
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_B, 20, 105);
                break;
            }
            console.log("Detected false positive (A) Next button.", COLOR_RED);
        case 6:
            console.log("Detected summary.");
            if (result == TeraResult::NO_DETECTION){
                context.wait_for(std::chrono::milliseconds(500));
                result = run_tera_summary(
                    env, console, context,
                    notification_nonshiny,
                    notification_shiny,
                    stop_on_shiny, screenshot,
                    stat_shinies
                );
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 7:
            console.log("Detected unexpected main menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 8:
            console.log("Detected overworld.");
            if (stop_on_shiny && result == TeraResult::NO_DETECTION){
                throw UserSetupError(
                    console,
                    "Unable to find " + STRING_POKEMON + " summary to check for shininess. "
                    "Make sure your party is full and \"Send to Boxes\" is set to \"Manual\"."
                );
            }
            return result;
        default:
            dump_image_and_throw_recoverable_exception(
                env.program_info(), console, "ExitTeraWinFailed",
                "exit_tera_win_by_catching(): No recognized state after 60 seconds."
            );
        }
    }
}




TeraResult run_tera_summary(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, const ImageViewRGB32& battle_screenshot,
    std::atomic<uint64_t>* stat_shinies
){
    console.log("Reading summary...");

    VideoSnapshot screen = console.video().snapshot();
    PokemonSummaryDetector reader;
    if (reader.is_shiny(screen)){
        if (stat_shinies != nullptr){
            (*stat_shinies)++;
        }
        send_encounter_notification(
            env,
            notification_nonshiny,
            notification_shiny,
            false, true,
            {{{}, ShinyType::UNKNOWN_SHINY}},
            std::nan(""),
            battle_screenshot
        );
        if (stop_on_shiny){
            throw ProgramFinishedException();
        }
        return TeraResult::SHINY;
    }else{
        send_encounter_notification(
            env,
            notification_nonshiny,
            notification_shiny,
            false, false,
            {{{}, ShinyType::NOT_SHINY}},
            std::nan("")
        );
        return TeraResult::NOT_SHINY;
    }
}


void run_from_tera_battle(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Running away from tera raid battle...");

    WallClock start = current_time();
    while (true){
        // Having a lot of Abilities activating can take a while, setting 3 minutes to be safe
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_from_tera_battle(): Failed to run away from tera raid battle after 3 minutes.",
                true
            );
        }

        TeraBattleMenuWatcher battle_menu(COLOR_GREEN);
        OverworldWatcher overworld(COLOR_CYAN);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::minutes(1),
            {battle_menu, overworld}
        );

        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected tera raid battle menu, running away...");
            console.overlay().add_log("Running away...", COLOR_WHITE);
            battle_menu.move_to_slot(console, context, 2);
            pbf_mash_button(context, BUTTON_A, 800);
            continue;
        case 1:
            console.log("Detected overworld.");
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "run_from_tera_battle(): No recognized state after 1 minutes.",
                true
            );
        }
    }
}


bool is_sparkling_raid(ConsoleHandle& console, BotBaseContext& context){
    OverworldWatcher static_map(COLOR_CYAN, true);
    context.wait_for_all_requests();

    int ret = wait_until(
        console, context,
        std::chrono::seconds(2),
        {static_map}
    );

    if (ret == 0){
        console.log("Did not detect sparkling raid", COLOR_ORANGE);
        return false;
    }
    console.log("Detected sparkling raid", COLOR_ORANGE);
    return true;
}









}
}
}
