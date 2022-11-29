/*  Clone Items
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_CloneItems-1.0.1.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


CloneItems101_Descriptor::CloneItems101_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:CloneItems1.0.1",
        STRING_POKEMON + " SV", "Clone Items (1.0.1)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/CloneItems-101.md",
        "Clone items using the add-to-party glitch.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct CloneItems101_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_cloned(m_stats["Cloned"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Cloned");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_cloned;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> CloneItems101_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




class BattleTeamDetector{
public:
    BattleTeamDetector()
        : m_slot1(0.024, 0.187 + 0 * 0.1166, 0.005, 0.100)
        , m_slot2(0.024, 0.187 + 1 * 0.1166, 0.005, 0.100)
        , m_slot3(0.024, 0.187 + 2 * 0.1166, 0.005, 0.100)
        , m_slot4(0.024, 0.187 + 3 * 0.1166, 0.005, 0.100)
        , m_slot5(0.024, 0.187 + 4 * 0.1166, 0.005, 0.100)
        , m_slot6(0.024, 0.187 + 5 * 0.1166, 0.005, 0.100)
    {}

    bool detect(const ImageViewRGB32& screen) const{
        ImageStats stats1 = image_stats(extract_box_reference(screen, m_slot1));
//        cout << stats0.average << stats0.stddev << endl;
        if (!is_white(stats1, 500, 20)) return false;
        ImageStats stats2 = image_stats(extract_box_reference(screen, m_slot2));
        if (!is_white(stats2, 500, 20)) return false;
        ImageStats stats3 = image_stats(extract_box_reference(screen, m_slot3));
        if (!is_white(stats3, 500, 20)) return false;
        ImageStats stats4 = image_stats(extract_box_reference(screen, m_slot4));
        if (!is_white(stats4, 500, 20)) return false;
        ImageStats stats5 = image_stats(extract_box_reference(screen, m_slot5));
        if (!is_white(stats5, 500, 20)) return false;
        ImageStats stats6 = image_stats(extract_box_reference(screen, m_slot6));
        if (!is_white(stats6, 500, 20)) return false;
        return true;
    }

private:
    ImageFloatBox m_slot1;
    ImageFloatBox m_slot2;
    ImageFloatBox m_slot3;
    ImageFloatBox m_slot4;
    ImageFloatBox m_slot5;
    ImageFloatBox m_slot6;
};



CloneItems101::CloneItems101()
    : GO_HOME_WHEN_DONE(false)
    , ITEMS_TO_CLONE(
        "<b>Items to Clone:</b><br>Clone this many time.",
        LockWhileRunning::UNLOCKED,
        999, 1, 999
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(ITEMS_TO_CLONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void CloneItems101::clone_item(ConsoleHandle& console, BotBaseContext& context){
    bool item_held = false;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to clone an item after 5 minutes.");
        }

        OverworldWatcher overworld(COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        GradientArrowWatcher party_select_top(COLOR_GREEN, console.overlay(), GradientArrowType::RIGHT, {0.30, 0.27, 0.10, 0.08});
        GradientArrowWatcher party_select_return(COLOR_GREEN, console.overlay(), GradientArrowType::RIGHT, {0.30, 0.57, 0.10, 0.08});
        GradientArrowWatcher party_select_back(COLOR_GREEN, console.overlay(), GradientArrowType::RIGHT, {0.30, 0.64, 0.10, 0.08});
        AdvanceDialogWatcher dialog(COLOR_CYAN);
        GradientArrowWatcher box_slot_one(COLOR_BLUE, console.overlay(), GradientArrowType::DOWN, {0.24, 0.16, 0.05, 0.09});

        PromptDialogDetector return_to_ride_prompt(COLOR_DARKGREEN, {0.500, 0.545, 0.400, 0.100});
        BattleTeamDetector battle_team;

        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {
                overworld,
                main_menu,
                party_select_top,
                party_select_return,
                party_select_back,
                dialog,
                box_slot_one,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected overworld. (unexpected)", COLOR_RED);
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (item_held){
                main_menu.move_cursor(console, context, MenuSide::RIGHT, 1);
                pbf_press_button(context, BUTTON_A, 20, 105);
            }else{
                main_menu.move_cursor(console, context, MenuSide::LEFT, 5);
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
            continue;
        case 2:
            console.log("Detected 6th slot select top.");
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        case 3:
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 4:
            console.log("Detected 6th slot select back. (unexpected)", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        case 5:{
            console.log("Detected dialog.");

            //  Resolve ambiguities.

            VideoSnapshot snapshot = console.video().snapshot();

            //  Confirmation prompt for returning your ride back to ride form.
            if (return_to_ride_prompt.detect(snapshot)){
                pbf_press_button(context, BUTTON_A, 20, 105);
                item_held = true;
                continue;
            }



            //  No other recognized ambiguities.
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        }
        case 6:{
            console.log("Detected box slot 1.");

            if (!item_held){
                pbf_press_button(context, BUTTON_B, 20, 105);
//                continue;
                return;
            }

            VideoSnapshot snapshot = console.video().snapshot();

            //  Not on the battle teams.
            if (!battle_team.detect(snapshot)){
                pbf_press_button(context, BUTTON_X, 20, 105);
                pbf_press_button(context, BUTTON_X, 20, 105);
                continue;
            }

            pbf_press_button(context, BUTTON_L, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_press_button(context, BUTTON_A, 20, 105);
            item_held = false;

            continue;
        }
        default:
            throw OperationFailedException(console.logger(), "No recognized state after 60 seconds.");
        }

    }
}

void CloneItems101::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    CloneItems101_Descriptor::Stats& stats = env.current_stats<CloneItems101_Descriptor::Stats>();

    for (uint16_t cloned = 0; cloned < ITEMS_TO_CLONE; cloned++){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        clone_item(env.console, context);
        stats.m_cloned++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
