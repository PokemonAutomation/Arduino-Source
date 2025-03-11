/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_TradeRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{




TradeStats::TradeStats()
    : m_trades(m_stats["Trades"])
    , m_errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
}


void trade_current_pokemon(
    VideoStream& stream, ProControllerContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(stream.logger());

    context.wait_for_all_requests();
    VideoSnapshot box_image = stream.video().snapshot();
    ImageMatchWatcher box_detector(std::move(box_image.frame), {0.02, 0.10, 0.15, 0.80}, 50);

//    pbf_press_button(context, BUTTON_ZL, 20, 0);

#if 0
    while (true){
        context.wait_for_all_requests();
        SelectionArrowFinder detector0(stream, {0.50, 0.58, 0.40, 0.10}, COLOR_RED);
        SelectionArrowFinder detector1(stream, {0.50, 0.52, 0.40, 0.10}, COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_ZL, 20 * TICKS_PER_SECOND);
            },
            {detector0, detector1}
        );
        switch (ret){
        case 0:
            stream.log("Detected trade prompt.");
            context.wait_for(std::chrono::milliseconds(100));
            tracker.check_unrecoverable_error(stream);
            pbf_press_button(context, BUTTON_ZL, 20, 0);
            continue;
        case 1:
            stream.log("Detected trade confirm prompt.");
            context.wait_for(std::chrono::milliseconds(100));
            tracker.check_unrecoverable_error(stream);
            pbf_press_button(context, BUTTON_ZL, 20, 0);
            break;
        default:
            stats.m_errors++;
            tracker.report_unrecoverable_error(stream, "Failed to detect a prompt after 20 seconds.");
        }
        break;
    }
#endif

    //  Start trade.
//    pbf_press_button(context, BUTTON_ZL, 20, 0);

    //  Wait for black screen.
    {
        BlackScreenOverWatcher black_screen;
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_ZL, 120 * TICKS_PER_SECOND);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(stream, "Failed to detect start of trade after 2 minutes.");
        }
        stream.log("Detected start of trade.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(stream.logger());
    }

    //  Mash B until 2nd black screen.
    {
        BlackScreenWatcher black_screen;
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(stream, "Failed to detect end of trade after 2 minutes.");
        }
        stream.log("Detected end of trade.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(stream.logger());
    }

    //  Wait to return to box.
    {
        int ret = wait_until(
            stream, context, std::chrono::minutes(2),
            {{box_detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(stream, "Failed to return to box after 2 minutes after a trade.");
        }
        stream.log("Detected box. Trade completed.");
        tracker.check_unrecoverable_error(stream.logger());
    }
}


void trade_current_box(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    EventNotificationOption& notifications,
    TradeStats& stats
){
    for (size_t row = 0; row < 5; row++){
        for (size_t col = 0; col < 6; col++){
            env.update_stats();
            send_program_status_notification(env, notifications);

            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                Milliseconds box_scroll_delay = GameSettings::instance().BOX_SCROLL_DELAY0;
                for (size_t r = 0; r < row; r++){
                    pbf_move_right_joystick(context, 128, 255, 160ms, box_scroll_delay);
                }
                for (size_t c = 0; c < col; c++){
                    pbf_move_right_joystick(context, 255, 128, 160ms, box_scroll_delay);
                }
                trade_current_pokemon(console, context, error_state, stats);
            });
            stats.m_trades++;
        }
    }
}





}
}
}
