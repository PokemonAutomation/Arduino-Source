/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_TradeRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




TradeStats::TradeStats()
    : m_trades(m_stats["Trades"])
    , m_errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
}


class TradeWarningDetector : public StaticScreenDetector{
public:
    TradeWarningDetector(Color color)
        : m_color(color)
//        , m_box_top(0.30, 0.33, 0.40, 0.02)
//        , m_box_bot(0.30, 0.65, 0.40, 0.02)
        , m_box_left(0.25, 0.33, 0.02, 0.34)
        , m_box_right(0.73, 0.33, 0.02, 0.34)
    {}
    virtual void make_overlays(VideoOverlaySet& items) const override{
//        items.add(m_color, m_box_top);
//        items.add(m_color, m_box_bot);
        items.add(m_color, m_box_left);
        items.add(m_color, m_box_right);
    }
    virtual bool detect(const ImageViewRGB32& screen) override{
        ImageStats box_top = image_stats(extract_box_reference(screen, m_box_left));
        if (!is_solid(box_top, {0.11424, 0.310539, 0.575221}, 0.30)){
            return false;
        }
        ImageStats box_bot = image_stats(extract_box_reference(screen, m_box_right));
        if (!is_solid(box_bot, {0.11424, 0.310539, 0.575221}, 0.30)){
            return false;
        }
        return true;
    }

protected:
    Color m_color;
//    ImageFloatBox m_box_top;
//    ImageFloatBox m_box_bot;
    ImageFloatBox m_box_left;
    ImageFloatBox m_box_right;
};
class TradeWarningFinder : public DetectorToFinder<TradeWarningDetector>{
public:
    TradeWarningFinder(Color color)
         : DetectorToFinder("TradeWarningFinder", std::chrono::milliseconds(250), color)
    {}
};



class TradeDoneDetector : public StaticScreenDetector{
public:
    TradeDoneDetector(VideoOverlay& overlay)
        : m_cursor(COLOR_RED, GradientArrowType::DOWN, {0.24, 0.17, 0.38, 0.55})
        , m_slot(COLOR_RED, true)
    {}
    virtual void make_overlays(VideoOverlaySet& items) const override{
        m_cursor.make_overlays(items);
        m_slot.make_overlays(items);
    }
    virtual bool detect(const ImageViewRGB32& screen) override{
        bool cursor_ok = m_cursor.detect(screen);
        bool slot_ok = m_slot.detect(screen);
//        cout << "cursor = " << cursor_ok << ", slot_ok = " << slot_ok << endl;
        return cursor_ok && slot_ok;
    }

private:
    GradientArrowDetector m_cursor;
    SomethingInBoxSlotDetector m_slot;
};
class TradeDoneHold : public DetectorToFinder<TradeDoneDetector>{
public:
    TradeDoneHold(VideoOverlay& overlay)
         : DetectorToFinder("AdvanceDialogHold", std::chrono::milliseconds(500), overlay)
    {}
};



void trade_current_pokemon(
    VideoStream& stream, ProControllerContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(stream.logger());
    context.wait_for_all_requests();

    //  Make sure there is something to trade.
    {
        SomethingInBoxSlotDetector detector(COLOR_CYAN, true);
        if (!detector.detect(stream.video().snapshot())){
            stats.m_errors++;
            tracker.report_unrecoverable_error(stream, "Box slot is empty.");
        }
    }


    //  Wait for black screen.
    {
        BlackScreenOverWatcher black_screen(COLOR_CYAN);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 120 * TICKS_PER_SECOND);
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


    TradeDoneHold trade_done(stream.overlay());

    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW, DialogType::DIALOG_ALL, std::chrono::seconds(2));
        PromptDialogWatcher learn_move(COLOR_BLUE);

        int ret = wait_until(
            stream, context, std::chrono::minutes(2),
            {dialog, trade_done, learn_move}
        );
        switch (ret){
        case 0:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 1:
            stream.log("Detected box. Trade completed.");
            tracker.check_unrecoverable_error(stream.logger());
            context.wait_for(std::chrono::milliseconds(500));
            return;
        case 2:
            stream.log("Detected move learn.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        default:
            stats.m_errors++;

            tracker.report_unrecoverable_error(stream, "Failed to return to box after 2 minutes after a trade.");
        }
    }
}


void trade_current_box(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    EventNotificationOption& notifications,
    TradeStats& stats,
    uint8_t start_row, uint8_t start_col
){
    for (uint8_t row = start_row; row < 5; row++){
        for (uint8_t col = start_col; col < 6; col++){
            env.update_stats();
            send_program_status_notification(env, notifications);

            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                VideoOverlaySet overlays(console.overlay());

                move_box_cursor(env.program_info(), console, context, BoxCursorLocation::SLOTS, row, col);

                trade_current_pokemon(console, context, error_state, stats);
            });
            stats.m_trades++;
        }
        start_col = 0;
    }
}




}
}
}
