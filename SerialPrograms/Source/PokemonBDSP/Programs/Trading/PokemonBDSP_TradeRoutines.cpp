/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP_TradeRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



void MultiConsoleErrorState::report_unrecoverable_error(Logger& logger, const std::string& msg){
    logger.log(msg, Qt::red);
    m_unrecoverable_error.store(true, std::memory_order_release);
    PA_THROW_StringException(msg);
}
void MultiConsoleErrorState::check_unrecoverable_error(Logger& logger){
    if (m_unrecoverable_error.load(std::memory_order_acquire)){
        logger.log("Unrecoverable error reported from a different console. Breaking out.", Qt::red);
        throw CancelledException();
    }
}



TradeStats::TradeStats()
    : m_trades(m_stats["Trades"])
    , m_errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Errors", true);
}


void trade_current_pokemon(
    ProgramEnvironment& env, ConsoleHandle& console,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(console);

    console.botbase().wait_for_all_requests();
    QImage box_image = console.video().snapshot();
    ImageMatchWatcher box_detector(box_image, {0.02, 0.10, 0.15, 0.80}, 50);

    {
        pbf_press_button(console, BUTTON_ZL, 20, 0);
        console.botbase().wait_for_all_requests();
        SelectionArrowFinder detector(console, {0.50, 0.58, 0.40, 0.10}, Qt::red);
        int ret = wait_until(
            env, console, std::chrono::seconds(120),
            { &detector }
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade select prompt after 2 minutes.");
        }
        console.log("Detected trade prompt.");
        env.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }
    {
        pbf_press_button(console, BUTTON_ZL, 20, 0);
        console.botbase().wait_for_all_requests();
        SelectionArrowFinder detector(console, {0.50, 0.52, 0.40, 0.10}, Qt::red);
        int ret = wait_until(
            env, console, std::chrono::seconds(10),
            { &detector }
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade confirm prompt after 10 seconds.");
        }
        console.log("Detected trade confirm prompt.");
        env.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Start trade.
    pbf_press_button(console, BUTTON_ZL, 20, 0);

    //  Wait for black screen.
    {
        BlackScreenOverWatcher black_screen;
        int ret = wait_until(
            env, console, std::chrono::minutes(2),
            { &black_screen }
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect start of trade after 2 minutes.");
        }
        console.log("Detected start of trade.");
        env.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Mash B until 2nd black screen.
    {
        BlackScreenWatcher black_screen;
        int ret = run_until(
            env, console,
            [](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            { &black_screen }
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect end of trade after 2 minutes.");
        }
        console.log("Detected end of trade.");
        env.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Wait to return to box.
    {
        int ret = wait_until(
            env, console, std::chrono::minutes(2),
            { &box_detector }
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to return to box after 2 minutes after a trade.");
        }
        console.log("Detected box. Trade completed.");
        tracker.check_unrecoverable_error(console);
    }
}


void trade_current_box(
    MultiSwitchProgramEnvironment& env,
    EventNotificationOption& notifications,
    TradeStats& stats
){
    for (size_t row = 0; row < 5; row++){
        for (size_t col = 0; col < 6; col++){
            env.update_stats();
            send_program_status_notification(
                env.logger(), notifications,
                env.program_info(),
                "",
                stats.to_str()
            );

            MultiConsoleErrorState error_state;
            env.run_in_parallel([&](ConsoleHandle& console){
                uint16_t box_scroll_delay = GameSettings::instance().BOX_SCROLL_DELAY_0;
                for (size_t r = 0; r < row; r++){
                    pbf_move_right_joystick(console, 128, 255, 20, box_scroll_delay);
                }
                for (size_t c = 0; c < col; c++){
                    pbf_move_right_joystick(console, 255, 128, 20, box_scroll_delay);
                }
                trade_current_pokemon(env, console, error_state, stats);
            });
            stats.m_trades++;
        }
    }
}





}
}
}
