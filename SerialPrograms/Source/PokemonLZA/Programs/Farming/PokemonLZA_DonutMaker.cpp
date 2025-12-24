/*  Donut Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_DonutMaker.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


DonutMaker_Descriptor::DonutMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:DonutMaker", STRING_POKEMON + " LZA",
        "Donut Maker",
        "Programs/PokemonLZA/DonutMaker.html",
        "TODO: Add description",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class DonutMaker_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        // TODO: Add more stats here
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> DonutMaker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DonutMaker::DonutMaker()
    : NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    // TODO: Add options here using PA_ADD_OPTION()
    PA_ADD_OPTION(NOTIFICATIONS);
}

// Press A to talk to Ansha and keep pressing A until reach the berry selection menu
void open_berry_menu_from_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    // press button A to start talking to Ansha
    pbf_press_button(context, BUTTON_A, 100ms, 200ms);
    context.wait_for_all_requests();
    
    WallClock start = current_time();
    while(current_time() - start <= Seconds(120)){
        FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &env.console.overlay());
        SelectionArrowWatcher arrow(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.591, 0.579, 0.231, 0.105}
        );
        DonutBerriesSelectionWatcher berry_selection(0);

        int ret = wait_until(env.console, context, std::chrono::seconds(3),
            {white_dialog, arrow, berry_selection});
        switch (ret){
        case 0:
            env.log("Detected white dialog. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 1:
            env.log("Detected selection arrow. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 2:
            env.log("Berry selection menu shown.");
            env.add_overlay_log("Found Berry Selection Menu");
            return;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Unable to detect white dialog, selection arrow or berry menu after talking to Ansha.",
                env.console
            );
        } // end switch(ret)
        context.wait_for_all_requests();
    } // end while(true)

    stats.errors++;
    env.update_stats();
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "donut_maker(): 2 minutes passed yet unable to reach berry menu after taking to Ansha.",
        env.console
    );
}


// Return true if it should stop
// Start the iteration at closest pokemon center
bool donut_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    bool zoom_to_max = false;
    open_map(env.console, context, zoom_to_max);
    // Move map cursor upwards a little bit
    pbf_move_left_joystick(context, 128, 64, 100ms, 200ms);
    // Press Y to load fast travel locaiton menu. The cursor should now points to Vert Pokemon Center
    pbf_press_button(context, BUTTON_Y, 125ms, 200ms);
    // Move one menu item up to select Hotel Z
    pbf_press_dpad(context, DPAD_UP, 125ms, 200ms);

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, Seconds(10));
            pbf_wait(context, Seconds(30));
        },
        {overworld}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find overworld after fast traveling from Vert Pokemon Cenetr after 30 sec.",
            env.console
        );
    }

    ret = run_towards_gate_with_A_button(env.console, context, 128, 0, Seconds(5));
    if (ret == 1){
        if (run_towards_gate_with_A_button(env.console, context, 128, 0, Seconds(5)) != 0){
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Cannot reach Hotel Z gate after day/night change.",
                env.console
            );
        }
    } else if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot reach Hotel Z gate after fast travel.",
            env.console
        );
    }

    // Mash button A to enter the hotel.
    pbf_mash_button(context, BUTTON_A, Seconds(2));
    context.wait_for_all_requests();
    WallClock start_time = current_time();
    // We use 50s here to account for day night change
    wait_until_overworld(env.console, context, 50s);
    env.console.log("Detected overworld after entering zone.");
    WallClock end_time = current_time();
    const auto duration = end_time - start_time;
    // Due to day/night change may eating the mashing button A sequence, we may still be outside the hotel!
    if (duration >= 16s){
        // mash A again to make sure we are inside the hotel
        pbf_mash_button(context, BUTTON_A, Seconds(2));
        context.wait_for_all_requests();
        wait_until_overworld(env.console, context, 50s);
    }

    // we are now inside the hotel

    // Roll forward twice
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_move_left_joystick(context, 0, 128, 500ms, 100ms);
    context.wait_for_all_requests();


    const ImageFloatBox button_A_box{0.3, 0.2, 0.4, 0.7};
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, button_A_box, &env.console.overlay());
    ret = wait_until(env.console, context, std::chrono::seconds(3), {buttonA});
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find button A facing Ansha.",
            env.console
        );
    }

    open_berry_menu_from_ansha(env, context);


    return true; // XXX

    return false;
}


void DonutMaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    assert_16_9_1080p_min(env.logger(), env.console);

    //  Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);


    while(true){
        const bool should_stop = donut_iteration(env, context);
        stats.resets++;
        env.update_stats();

        if (should_stop){
            break;
        }
    }
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
