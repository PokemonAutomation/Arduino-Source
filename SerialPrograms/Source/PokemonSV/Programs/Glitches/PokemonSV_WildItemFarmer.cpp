/*  Wild Item Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_WildItemFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



WildItemFarmer_Descriptor::WildItemFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:WildItemFarmer",
        Pokemon::STRING_POKEMON + " SV", "Wild Item Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/WildItemFarmer.md",
        "Farm an item held by a wild " + Pokemon::STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


struct WildItemFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : battles(m_stats["Battles"])
        , items(m_stats["Items Cloned"])
        , failed(m_stats["Clone Failed"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Items Cloned");
        m_display_order.emplace_back("Clone Failed", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& items;
    std::atomic<uint64_t>& failed;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> WildItemFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




WildItemFarmer::WildItemFarmer()
    : ITEMS_TO_CLONE(
        "<b>Items to Clone:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        999, 0, 999
    )
#if 0
    , TRICK_MOVE_SLOT(
        "<b>Trick Move Slot:",
        {
            {0, "slot1", "Slot 1"},
            {1, "slot2", "Slot 2"},
            {2, "slot3", "Slot 3"},
            {3, "slot4", "Slot 4"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
#endif
    , INITIAL_TRICK_PP(
        "<b>Initial Trick PP:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0, 16
    )
    , VERIFY_ITEM_CLONED(
        "<b>Verify Item Cloned:</b><br>Verify each run that the item has actually been cloned. "
        "This will slow each iteration by a few seconds, but will better detect errors.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , ENABLE_FORWARD_RUN(
        "<b>Forward Run:</b><br>Run forward a bit before throwing ball. This will correct for "
        "the clone moving away, but may cause your position to wander more.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(ITEMS_TO_CLONE);
//    PA_ADD_OPTION(TRICK_MOVE_SLOT);
    PA_ADD_OPTION(INITIAL_TRICK_PP);
    PA_ADD_OPTION(VERIFY_ITEM_CLONED);
    PA_ADD_OPTION(ENABLE_FORWARD_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void WildItemFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    try{
        run_program(env, context);
    }catch (...){
        pbf_press_button(context, BUTTON_HOME, 20, 105);
        throw;
    }
}

void WildItemFarmer::refresh_pp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    int move_overwrites = 0;
    bool move_selected = false;
    while (true){
        WhiteButtonWatcher summary(
            COLOR_RED, WhiteButton::ButtonA,
            {0.12, 0.67, 0.20, 0.06},
            WhiteButtonWatcher::FinderType::PRESENT,
            std::chrono::milliseconds(500)
        );
        AdvanceDialogWatcher dialog(COLOR_RED);
        GradientArrowWatcher action_select(COLOR_GREEN, GradientArrowType::RIGHT, {0.515, 0.33, 0.05, 0.09});
        GradientArrowWatcher move_select(COLOR_YELLOW, GradientArrowType::RIGHT, {0.05, 0.20, 0.05, 0.09});
        context.wait_for_all_requests();

        int ret = wait_until(
            env.console, context, std::chrono::seconds(10),
            {
                summary,
                dialog,
                action_select,
                move_select,
            }
        );
        if (move_overwrites >= 2){
            pbf_mash_button(context, BUTTON_B, 50);
            return;
        }

        switch (ret){
        case 0:
            if (move_selected){
                move_overwrites++;
            }
            move_selected = false;
            env.log("Detected Change Moves button. Overwrite count = " + std::to_string(move_overwrites));
            if (move_overwrites >= 2){
                continue;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;

        case 1:
            env.log("Detected dialog.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;

        case 2:
            env.log("Detected action select.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;

        case 3:
            env.log("Detected move select.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            move_selected = true;
            continue;

        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No state detected while changing moves after 10 seconds.",
                env.console
            );
        }
    }
}

bool WildItemFarmer::verify_item_held(SingleSwitchProgramEnvironment& env, ProControllerContext& context, NormalBattleMenuWatcher& battle_menu){
    env.log("Verifying that item has been taken...");

    while (true){
        SwapMenuWatcher swap_menu(COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context, std::chrono::seconds(10),
            {battle_menu, swap_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            env.log("Detected battle menu...");
            if (!battle_menu.move_to_slot(env.console, context, 1)){
                return false;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;

        case 1:
            env.log("Detected swap menu...");
            break;

        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to detect " + Pokemon::STRING_POKEMON + " select menu.",
                env.console
            );
        }

        break;
    }

    VideoSnapshot screen = env.console.video().snapshot();
    ImageViewRGB32 box = extract_box_reference(screen, ImageFloatBox(0.28, 0.20, 0.03, 0.055));
    ImageStats stats = image_stats(box);
    bool item_held = !is_solid(stats, {0.550405, 0.449595, 0.}, 0.20);

    {
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 500);
            },
            {battle_menu}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to back out to battle menu.",
                env.console
            );
        }
    }

    return item_held;
}


void WildItemFarmer::run_program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    WildItemFarmer_Descriptor::Stats& stats = env.current_stats<WildItemFarmer_Descriptor::Stats>();

    const std::vector<std::pair<int, int>> MANUVERS{
        {128, 0},
        {96, 0},
        {160, 0},
    };

    uint16_t items_cloned = 0;
    bool trick_used = false;
    bool overworld_seen = false;
    int8_t trick_PP = INITIAL_TRICK_PP;
    uint8_t consecutive_throw_attempts = 0;
    WallClock last_trick_attempt = WallClock::min();
    while (items_cloned < ITEMS_TO_CLONE){
        OverworldWatcher overworld(env.console, COLOR_CYAN);
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        MoveSelectWatcher move_select(COLOR_YELLOW);
        MainMenuWatcher main_menu(COLOR_GREEN);
        PokemonSummaryWatcher summary(COLOR_BLUE);

        context.wait_for_all_requests();

        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                battle_menu,
                move_select,
                main_menu,
                summary,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            env.log("Detected overworld.");
            if (trick_used && trick_PP > 0){
                trick_PP--;
                if (!VERIFY_ITEM_CLONED){
                    items_cloned++;
                    stats.items++;
                    env.update_stats();
                }
            }
            if (!overworld_seen){
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            }
            overworld_seen = true;
            trick_used = false;

            env.log("Trick PP: " + std::to_string(trick_PP));

            if (trick_PP <= 0){
                pbf_press_button(context, BUTTON_X, 20, 105);
                continue;
            }

            if (consecutive_throw_attempts >= MANUVERS.size()){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    "Failed to start battle after " + std::to_string(MANUVERS.size()) + " attempts.",
                    env.console
                );
            }

            pbf_press_button(context, BUTTON_L, 20, 23);
            if (ENABLE_FORWARD_RUN){
                const std::pair<int, int>& direction = MANUVERS[consecutive_throw_attempts];
                pbf_move_left_joystick(context, (uint8_t)direction.first, (uint8_t)direction.second, 50, 0);
            }
            pbf_mash_button(context, BUTTON_ZR, 250);
            pbf_wait(context, 350);

            consecutive_throw_attempts++;

            continue;

        case 1:
            env.log("Detected battle menu.");
            if (current_time() - std::chrono::seconds(5) < last_trick_attempt){
                env.log("Unable to use move. Assume out of PP.");
                trick_PP = 0;
//                pbf_mash_button(context, BUTTON_B, 30);
//                continue;
            }

            consecutive_throw_attempts = 0;
            if (overworld_seen){
                stats.battles++;
                env.update_stats();
            }
            overworld_seen = false;

            if (trick_used && trick_PP > 0 && VERIFY_ITEM_CLONED){
                if (verify_item_held(env, context, battle_menu)){
                    items_cloned++;
                    stats.items++;
                    env.update_stats();
                }else{
                    stats.failed++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::NO_ERROR_REPORT,
                        "Failed to clone item. Possible incorrect encounter.",
                        env.console
                    );
                }
            }

            if (trick_used || trick_PP <= 0){
                env.log("Running away...");
                if (battle_menu.move_to_slot(env.console, context, 3)){
                    pbf_press_button(context, BUTTON_A, 20, 30);
                }else{
                    stats.errors++;
                    env.update_stats();
                    pbf_mash_button(context, BUTTON_B, 125);
                }
            }else{
                env.log("Attempt to select a move.");
                if (battle_menu.move_to_slot(env.console, context, 0)){
                    pbf_press_button(context, BUTTON_A, 20, 30);
                }else{
                    stats.errors++;
                    env.update_stats();
                    pbf_mash_button(context, BUTTON_B, 125);
                }
            }
            continue;

        case 2:
            env.log("Detected move select.");
            if (current_time() - std::chrono::seconds(5) < last_trick_attempt){
                env.log("Unable to use move. Assume out of PP.");
                trick_PP = 0;
                pbf_mash_button(context, BUTTON_B, 30);
                continue;
            }
            if (move_select.move_to_slot(env.console, context, 0)){
                pbf_press_button(context, BUTTON_A, 20, 30);
                trick_used = true;
                last_trick_attempt = current_time();
            }else{
                stats.errors++;
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 125);
            }
            continue;

        case 3:
            env.log("Detected main menu.");
            if (trick_PP > 0){
                env.log("Backing out to overworld...");
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            }

            if (!main_menu.move_cursor(env.program_info(), env.console, context, MenuSide::LEFT, 0)){
                stats.errors++;
                env.update_stats();
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            }

            env.log("Attempting to enter summary...");

            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);

            continue;

        case 4:
            env.log("Detected " + Pokemon::STRING_POKEMON + " summary.");

            if (trick_PP > 0){
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            }

            pbf_press_dpad(context, DPAD_RIGHT, 20, 230);

            refresh_pp(env, context);
            trick_PP = 10;

            pbf_press_button(context, BUTTON_B, 20, 105);

//            throw ProgramFinishedException(env.console, "Out of PP.");
            continue;

        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No state detected after 120 seconds.",
                env.console
            );
        }


    }


    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
