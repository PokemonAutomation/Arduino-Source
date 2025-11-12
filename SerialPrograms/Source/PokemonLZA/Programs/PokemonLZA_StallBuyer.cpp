/*  Stall Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA_StallBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

StallBuyer_Descriptor::StallBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:StallBuyer",
        STRING_POKEMON + " LZA", "Stall Buyer",
        "Programs/PokemonLZA/StallBuyer.html",
        "Buy berries or mints from stall.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class StallBuyer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : purchases(m_stats["Purchases"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Purchases");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& purchases;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StallBuyer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


StallBuyer::StallBuyer()
    : ITEM_POSITION(
        "<b>Item Position to Purchase:</b>",
        {
            {ItemPosition::FirstItem,   "FirstItem",    "First Item"    },
            {ItemPosition::SecondItem,  "SecondItem",   "Second Item"   },
            {ItemPosition::ThirdItem,   "ThirdItem",    "Third Item"    },
            {ItemPosition::FourthItem,  "FourthItem",   "Fourth Item"   },
            {ItemPosition::FifthItem,   "FifthItem",    "Fifth Item"    },
            {ItemPosition::SixthItem,   "SixthItem",    "Sixth Item"    },
            {ItemPosition::SeventhItem, "SeventhItem",  "Seventh Item"  },
        },
        LockMode::LOCK_WHILE_RUNNING,
        ItemPosition::FirstItem
    )
    , NUM_PURCHASE(
        "<b>Number to Purchase:</b><br>The number of items you want to purchase.",
        LockMode::LOCK_WHILE_RUNNING,
        100, 1, 999
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(ITEM_POSITION);
    PA_ADD_OPTION(NUM_PURCHASE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

int detect_stall_amount_item(SingleSwitchProgramEnvironment& env, StallBuyer_Descriptor::Stats& stats){
    // When buying from a stall, the first item is always selected.
    // Detect which one is currently selected (with white background)
    // 0.700 as y is the bottom option, then each one is shifted by 0.072
    ImageFloatBox seven_item_stall_box  (0.858, 0.700 - 7 * 0.072, 0.024, 0.019);
    ImageFloatBox six_item_stall_box    (0.858, 0.700 - 6 * 0.072, 0.024, 0.019);
    ImageFloatBox five_item_stall_box   (0.858, 0.700 - 5 * 0.072, 0.024, 0.019);
    ImageFloatBox two_item_stall_box    (0.858, 0.700 - 2 * 0.072, 0.024, 0.019);

    WhiteScreenDetector seven_item_stall_detector(COLOR_BLUE, seven_item_stall_box);
    WhiteScreenDetector six_item_stall_detector(COLOR_BLUE, six_item_stall_box);
    WhiteScreenDetector five_item_stall_detector(COLOR_BLUE, five_item_stall_box);
    WhiteScreenDetector two_item_stall_detector(COLOR_BLUE, two_item_stall_box);

    VideoSnapshot snapshot = env.console.video().snapshot();
    bool is_seven_item_stall = seven_item_stall_detector.detect(snapshot);
    bool is_six_item_stall = six_item_stall_detector.detect(snapshot);
    bool is_five_item_stall = five_item_stall_detector.detect(snapshot);
    bool is_two_item_stall = two_item_stall_detector.detect(snapshot);

    int count = is_seven_item_stall + is_six_item_stall + is_five_item_stall + is_two_item_stall;
    if (count == 1){
        // Exactly one kind of stall detected
        if (is_seven_item_stall){
            return 7;
        }else if (is_six_item_stall){
            return 6;
        }else if (is_five_item_stall){
            return 5;
        }else{
            return 2;
        }
    }else{
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "No recognized stall size.",
            env.console
        );
    }
}

std::pair<DpadPosition, int> compute_needed_inputs(int item_position, int stall_amount_item){
    int down_presses = item_position;
    int up_presses = stall_amount_item - item_position + 1;

    if (down_presses <= up_presses){
        return { DPAD_DOWN, down_presses };
    }else{
        return { DPAD_UP, up_presses };
    }
}

void StallBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StallBuyer_Descriptor::Stats& stats = env.current_stats<StallBuyer_Descriptor::Stats>();
    assert_16_9_720p_min(env.logger(), env.console);

    while (true) {
        context.wait_for_all_requests();

        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        SelectionArrowWatcher select(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.715, 0.165, 0.045, 0.440}
        );
        SelectionArrowWatcher confirm(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.665, 0.600, 0.145, 0.080}
        );
        FlatWhiteDialogWatcher dialog(COLOR_RED, &env.console.overlay());

        int ret = wait_until(
            env.console, context,
            30000ms,
                {
                    buttonA,
                    select,
                    confirm,
                    dialog,
                }
            );
        context.wait_for(100ms);

        switch (ret){
            case 0:
                env.log("Detected A button.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;

            case 1:
            {
                env.log("Detected item selection screen.");
                int stall_amount_item = detect_stall_amount_item(env, stats);
                env.log("Detected stall with " + std::to_string(stall_amount_item) + " items to sell.");
                auto [direction, presses] = compute_needed_inputs(static_cast<int>(ITEM_POSITION.get()), stall_amount_item);
                for (int i = 0; i < presses; i++){
                    pbf_press_dpad(context, direction, 160ms, 80ms);
                }
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;
            }
            case 2:
                env.log("Detected purchase confirm screen.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                stats.purchases++;
                env.update_stats();

                if (stats.purchases == NUM_PURCHASE) {
                    // pbf_mash_button(context, BUTTON_B, 5000ms);
                    // intentionally don't leave the purchase menu to not get attacked
                    GO_HOME_WHEN_DONE.run_end_of_program(context);
                    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
                    return;
                }

                continue;

            case 3:
                env.log("Detected white dialog.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;

            default:
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "No recognized state after 30 seconds.",
                    env.console
                );
        }
    }
}


}
}
}

