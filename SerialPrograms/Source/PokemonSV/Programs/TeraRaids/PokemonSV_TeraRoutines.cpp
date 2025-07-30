/*  Tera Exit Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokePortalDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRewardsReader.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV_TeraRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



bool open_raid(VideoStream& stream, ProControllerContext& context){
    stream.log("Opening raid...");
    while (true){
        TeraCardWatcher card_detector(COLOR_RED);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
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
            stream.log("Tera raid found!", COLOR_BLUE);
            return true;
        case 1:
            stream.log("Detect possible uncatchable dialog...", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            stream.log("No Tera raid found.", COLOR_ORANGE);
            return false;
        }
    }
}
void close_raid(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Closing raid...");

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "CloseRaidFailed",
                "Failed to return to overworld after 5 minutes."
            );
        }

        TeraCardWatcher card_detector(COLOR_RED);
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {card_detector, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            stream.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(info, stream, "CloseRaidFailed",
                "close_raid(): No recognized state after 60 seconds.");
        }
    }
}



void open_hosting_lobby(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    HostingMode mode
){
    bool recovery_mode = false;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(2)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), stream, "OpenLobbyFailed",
                "Unable to open Tera lobby after 2 minutes."
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        if (recovery_mode){
            context.wait_for_all_requests();
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_press_button(context, BUTTON_B, 20, 980);
                },
                {overworld}
            );
            if (ret < 0){
                continue;
            }
            stream.log("Detected overworld. Recovery finished.");
            recovery_mode = true;
        }

//        AdvanceDialogWatcher dialog(COLOR_GREEN);
        TeraCardWatcher card_detector(COLOR_YELLOW);
        TeraLobbyWatcher lobby(stream.logger(), COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {
                overworld,
//                dialog,
                card_detector,
                {lobby, std::chrono::milliseconds(500)}
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            recovery_mode = false;
            if (!open_raid(stream, context)){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "No Tera raid found.",
                    stream
                );
            }
            continue;
#if 0
        case 1:
            stream.log("Detect possible uncatchable dialog...", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_B, 20, 230);
            continue;
#endif
        case 1:
            stream.log("Detected Tera card.");
            if (mode != HostingMode::LOCAL){
                pbf_press_button(context, BUTTON_A, 20, 230);
                if (mode == HostingMode::ONLINE_EVERYONE){
                    pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                }
            }
            pbf_press_button(context, BUTTON_A, 20, 230);
            continue;
        case 2:
            stream.log("Detected Tera lobby.");
            return;
        default:
            stream.log("No state detected after 30 seconds. Backing out...", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 230);
            recovery_mode = true;
        }
    }
}





void enter_tera_search(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    bool connect_to_internet
){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "EnterTeraSearchFailed",
                "enter_tera_search(): Failed to enter Tera search."
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        PokePortalWatcher poke_portal(COLOR_GREEN);
        TeraRaidSearchWatcher raid_search(COLOR_CYAN, std::chrono::milliseconds(500));
        CodeEntryWatcher code_entry(COLOR_PURPLE);
        AdvanceDialogWatcher dialog(COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
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
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            stream.log("Detected main menu.");
            if (connect_to_internet && !connected){
                connect_to_internet_from_menu(info, stream, context);
                connected = true;
                continue;
            }
            if (main_menu.move_cursor(info, stream, context, MenuSide::RIGHT, 3)){
                pbf_press_button(context, BUTTON_A, 20, 230);
            }
            continue;
        case 2:
            stream.log("Detected Poke Portal.");
            if (poke_portal.move_cursor(info, stream, context, 1)){
                pbf_press_button(context, BUTTON_A, 20, 230);
            }
            continue;
        case 3:
            stream.log("Detected Tera Raid Search.");
            if (raid_search.move_cursor_to_search(info, stream, context)){
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
            continue;
        case 4:
            stream.log("Detected Code Entry.");
            return;
        case 5:
            stream.log("Detected Dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            dump_image_and_throw_recoverable_exception(
                info, stream, "EnterTeraSearchFailed",
                "enter_tera_search(): No recognized state after 30 seconds."
            );
        }
    }
}

void join_raid(
    const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context,
    bool connect_to_internet,
    KeyboardLayout keyboard_layout,
    RaidWaiter& raid_waiter
){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "JoinRaidFailed",
                "join_raid(): Failed to enter Tera raid."
            );
        }

        OverworldWatcher overworld(console.logger(), COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        PokePortalWatcher poke_portal(COLOR_GREEN);
        TeraRaidSearchWatcher raid_search(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_BLUE);
        CodeEntryWatcher code_entry(COLOR_PURPLE);
        TeraLobbyWatcher lobby(console.logger(), COLOR_ORANGE);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                poke_portal,
                raid_search,
                dialog,
                code_entry,
                lobby,
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
            console.log("Detected Dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;

        case 5:{
            console.log("Detected Code Entry.");
            std::string code = raid_waiter.wait_for_raid_code();
            enter_code(
                console, context,
                keyboard_layout,
                code,
                false, true, false
            );

#if 0
            if (console.index() == 1){
                dump_image_and_throw_recoverable_exception(
                    info, console, "InjectedError",
                    "join_raid(): Injected Error"
                );
            }
#endif

            break;
        }
        case 6:
            console.log("Detected Raid Lobby.");
            return;

        default:
            auto screen = console.video().snapshot();
            report_error(
                &console.logger(),
                info,
                "join_raid()",
                {{"Message", "No recognized state after 30 seconds."}},
                screen,
                &console.history()
            );

            pbf_press_button(context, BUTTON_B, 160ms, 840ms);
//            OperationFailedException::fire(
//                ErrorReport::SEND_ERROR_REPORT,
//                "join_raid(): No recognized state after 30 seconds.",
//                console,
//                std::move(screen)
//            );
        }
    }
}






void stop_if_enough_rare_items(
    VideoStream& stream, ProControllerContext& context,
    size_t stop_on_sparkly_items
){
    if (stop_on_sparkly_items == 0){
        return;
    }
    size_t sparkly_items = SparklyItemDetector::count_sparkly_items(stream, context);
    stream.log("Sparkly Items Detected: " + std::to_string(sparkly_items), COLOR_BLUE);
    if (sparkly_items >= stop_on_sparkly_items){
        throw_and_log<ProgramFinishedException>(
            stream.logger(),
            "Found a raid with " + std::to_string(sparkly_items) + " rare items!",
            stream
        );
    }
}


void exit_tera_win_without_catching(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t stop_on_sparkly_items
){
    stream.log("Exiting raid without catching...");

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "ExitTeraWinFailed",
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
        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {catch_menu, next_button, dialog, overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            stream.log("Detected catch prompt.");
            catch_menu.move_to_slot(stream, context, 1);
//            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_mash_button(context, BUTTON_A, 30);
            pbf_mash_button(context, BUTTON_B, 125);
            continue;
        case 1:
            stream.log("Detected possible (A) Next button.");
            stop_if_enough_rare_items(stream, context, stop_on_sparkly_items);
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 3:
            stream.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(
                info, stream, "ExitTeraWinFailed",
                "exit_tera_win_without_catching(): No recognized state after 60 seconds."
            );
        }
    }
}


void exit_tera_win_by_catching(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug,
    size_t stop_on_sparkly_items
){
    stream.log("Exiting raid with catching...");

    VideoSnapshot screenshot;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), stream, "ExitTeraWinFailed",
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
        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
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
            stream.log("Detected catch prompt.");
            screenshot = stream.video().snapshot();

            catch_menu.move_to_slot(stream, context, 0);
            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(stream, language);
            int quantity = move_to_ball(reader, stream, context, ball_slug);
            if (quantity == 0){
                throw_and_log<FatalProgramException>(
                    stream.logger(), ErrorReport::NO_ERROR_REPORT,
                    "Unable to find appropriate ball. Did you run out?",
                    stream
                );
            }
            if (quantity < 0){
                stream.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 1:
            stream.log("Detected (A) Next button.");
            stop_if_enough_rare_items(stream, context, stop_on_sparkly_items);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            stream.log("Detected add-to-party prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 4:
            stream.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 5:
            stream.log("Detected unexpected main menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 6:
            stream.log("Detected overworld.");
            return;
        default:
            dump_image_and_throw_recoverable_exception(
                env.program_info(), stream, "ExitTeraWinFailed",
                "exit_tera_win_by_catching(): No recognized state after 60 seconds."
            );
        }
    }
}


TeraResult exit_tera_win_by_catching(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, size_t stop_on_sparkly_items,
    std::atomic<uint64_t>* stat_shinies
){
    stream.log("Exiting raid with catching...");

    TeraResult result = TeraResult::NO_DETECTION;
    VideoSnapshot screenshot;
    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                env.program_info(), stream, "ExitTeraWinFailed",
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
        OverworldWatcher overworld(stream.logger(), COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
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
            stream.log("Detected catch prompt.");
            screenshot = stream.video().snapshot();

            catch_menu.move_to_slot(stream, context, 0);
            pbf_press_button(context, BUTTON_A, 20, 150);
            context.wait_for_all_requests();

            BattleBallReader reader(stream, language);
            int quantity = move_to_ball(reader, stream, context, ball_slug);
            if (quantity == 0){
                throw_and_log<FatalProgramException>(
                    stream.logger(), ErrorReport::NO_ERROR_REPORT,
                    "Unable to find appropriate ball. Did you run out?",
                    stream
                );
            }
            if (quantity < 0){
                stream.log("Unable to read ball quantity.", COLOR_RED);
            }
            pbf_mash_button(context, BUTTON_A, 125);

            continue;
        }
        case 2:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            stream.log("Detected add-to-party prompt.");
            if (result == TeraResult::NO_DETECTION){
                pbf_press_dpad(context, DPAD_DOWN, 20, 60);
//                pbf_press_button(context, BUTTON_A, 20, 105);
            }else{
                pbf_press_button(context, BUTTON_B, 20, 105);
            }
            continue;
        case 4:
            stream.log("Detected cursor over view summary.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 5:
            stream.log("Detected nickname prompt.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            //  Next button detector is unreliable. Check if the summary is
            //  open. If so, fall-through to that.
            if (!summary.detect(stream.video().snapshot())){
                stream.log("Detected possible (A) Next button.");
                stop_if_enough_rare_items(stream, context, stop_on_sparkly_items);
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_B, 20, 105);
                break;
            }
            stream.log("Detected false positive (A) Next button.", COLOR_RED);
        case 6:
            stream.log("Detected summary.");
            if (result == TeraResult::NO_DETECTION){
                context.wait_for(std::chrono::milliseconds(500));
                result = run_tera_summary(
                    env, stream, context,
                    notification_nonshiny,
                    notification_shiny,
                    stop_on_shiny, screenshot,
                    stat_shinies
                );
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 7:
            stream.log("Detected unexpected main menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 8:
            stream.log("Detected overworld.");
            if (stop_on_shiny && result == TeraResult::NO_DETECTION){
                throw UserSetupError(
                    stream.logger(),
                    "Unable to find " + STRING_POKEMON + " summary to check for shininess. "
                    "Make sure your party is full and \"Send to Boxes\" is set to \"Manual\"."
                );
            }
            return result;
        default:
            dump_image_and_throw_recoverable_exception(
                env.program_info(), stream, "ExitTeraWinFailed",
                "exit_tera_win_by_catching(): No recognized state after 60 seconds."
            );
        }
    }
}




TeraResult run_tera_summary(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, const ImageViewRGB32& battle_screenshot,
    std::atomic<uint64_t>* stat_shinies
){
    stream.log("Reading summary...");

    VideoSnapshot screen = stream.video().snapshot();
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


void run_from_tera_battle(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    std::atomic<uint64_t>* stat_errors
){
    stream.log("Running away from tera raid battle...");

    WallClock start = current_time();
    while (true){
        // Having a lot of Abilities activating can take a while, setting 3 minutes to be safe
        if (current_time() - start > std::chrono::minutes(3)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_from_tera_battle(): Failed to run away from tera raid battle after 3 minutes.",
                stream
            );
        }

        TeraBattleMenuWatcher battle_menu(COLOR_GREEN);
        GradientArrowWatcher leave_confirm(
            COLOR_RED,
            GradientArrowType::RIGHT,
            {0.557621, 0.471074, 0.25, 0.247934}
        );
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        TeraCardWatcher tera_card(COLOR_BLUE);
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::minutes(1),
            {
                battle_menu,
                leave_confirm,
                overworld,
                tera_card,
            }
        );

        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            stream.log("Detected tera raid battle menu, running away...");
            stream.overlay().add_log("Running away...", COLOR_WHITE);
            battle_menu.move_to_slot(stream, context, 2);
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;
        case 1:
            stream.log("Detected leave confirm.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;
        case 2:
            stream.log("Detected overworld.");
            return;
        case 3:
            stream.log("Detected a raid. (unexpected)", COLOR_RED);
            if (stat_errors){
                (*stat_errors)++;
                env.update_stats();
            }
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;
        default:
            if (stat_errors){
                (*stat_errors)++;
                env.update_stats();
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_from_tera_battle(): No recognized state after 1 minutes.",
                stream
            );
        }
    }
}




bool is_sparkling_raid(VideoStream& stream, ProControllerContext& context){
//    cout << "is_sparkling_raid()" << endl;

    FrozenImageDetector static_map(
        COLOR_RED,
        {0.890, 0.800, 0.030, 0.060},
        std::chrono::seconds(1), 20
    );

    context.wait_for_all_requests();

    int ret = wait_until(
        stream, context,
        std::chrono::seconds(2),
        {static_map}
    );

    if (ret == 0){
        stream.log("Did not detect sparkling raid", COLOR_ORANGE);
        return false;
    }
    stream.log("Detected sparkling raid", COLOR_ORANGE);
    return true;
}









}
}
}
