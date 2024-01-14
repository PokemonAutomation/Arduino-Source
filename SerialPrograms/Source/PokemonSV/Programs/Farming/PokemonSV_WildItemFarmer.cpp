/*  Wild Item Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
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
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


struct WildItemFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : battles(m_stats["Battles"])
        , items(m_stats["Items Cloned"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Items Cloned");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& items;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> WildItemFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




WildItemFarmer::WildItemFarmer()
    : ITEMS_TO_CLONE(
        "<b>Items to Clone:",
        LockMode::UNLOCK_WHILE_RUNNING,
        999, 0, 999
    )
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
    , INITIAL_TRICK_PP(
        "<b>Initial Trick PP:",
        LockMode::UNLOCK_WHILE_RUNNING,
        10, 0, 16
    )
{
    PA_ADD_OPTION(ITEMS_TO_CLONE);
    PA_ADD_OPTION(TRICK_MOVE_SLOT);
    PA_ADD_OPTION(INITIAL_TRICK_PP);
}


void WildItemFarmer::refresh_pp(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    int move_overwrites = 0;
    bool move_selected = false;
    while (true){
        WhiteButtonWatcher summary(
            COLOR_RED, WhiteButton::ButtonA,
            {0.22, 0.67, 0.03, 0.06},
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
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.logger(),
                "No state detected while changing moves after 10 seconds."
            );
        }
    }

//    PromptDialogWatcher remember_prompt(COLOR_BLUE);


}

void WildItemFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    WildItemFarmer_Descriptor::Stats& stats = env.current_stats<WildItemFarmer_Descriptor::Stats>();

    uint16_t items_cloned = 0;

    bool trick_used = false;
    bool overworld_seen = false;
    int8_t trick_PP = INITIAL_TRICK_PP;
    WallClock last_trick_attempt = WallClock::min();
    while (true){
        OverworldWatcher overworld(COLOR_CYAN);
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
                items_cloned++;
                stats.items++;
                env.update_stats();
            }
            overworld_seen = true;
            trick_used = false;

            env.log("Trick PP: " + std::to_string(trick_PP));

            if (trick_PP <= 0){
                pbf_press_button(context, BUTTON_X, 20, 105);
//                throw ProgramFinishedException(env.console, "Out of PP.");
                continue;
            }

            pbf_press_button(context, BUTTON_L, 20, 23);
            pbf_mash_button(context, BUTTON_ZR, 250);
            pbf_wait(context, 350);
            continue;

        case 1:
            env.log("Detected battle menu.");
            if (overworld_seen){
                stats.battles++;
                env.update_stats();
            }
            overworld_seen = false;

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
            if (move_select.move_to_slot(env.console, context, (uint8_t)TRICK_MOVE_SLOT.current_value())){
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
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.logger(),
                "No state detected after 120 seconds."
            );
        }


    }


}




}
}
}
