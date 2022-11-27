/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
//#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "PokemonSV_TradeRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




TradeStats::TradeStats()
    : m_trades(m_stats["Trades"])
    , m_errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Errors", true);
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
    virtual bool detect(const ImageViewRGB32& screen) const override{
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
    virtual bool detect(const ImageViewRGB32& screen) const override{
        return m_cursor.detect(screen) && m_slot.detect(screen);
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
    ConsoleHandle& console, BotBaseContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(console);
    context.wait_for_all_requests();

    //  Make sure there is something to trade.
    {
        SomethingInBoxSlotDetector detector(COLOR_CYAN, true);
        if (!detector.detect(console.video().snapshot())){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Box slot is empty.");
        }
    }

    {
        pbf_press_button(context, BUTTON_A, 20, 0);
        context.wait_for_all_requests();
        GradientArrowWatcher detector(COLOR_CYAN, console, GradientArrowType::RIGHT, {0.30, 0.18, 0.38, 0.08});
        int ret = wait_until(
            console, context, std::chrono::seconds(10),
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade select prompt after 10 seconds.");
        }
        console.log("Detected trade prompt.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }
    {
        pbf_press_button(context, BUTTON_A, 20, 0);
        context.wait_for_all_requests();
        PromptDialogWatcher detector(COLOR_CYAN, {0.500, 0.455, 0.400, 0.100});
        int ret = wait_until(
            console, context, std::chrono::seconds(30),
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade confirm prompt after 30 seconds.");
        }
        console.log("Detected trade confirm prompt.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }
    {
        pbf_press_button(context, BUTTON_A, 20, 0);
        context.wait_for_all_requests();
        TradeWarningFinder detector(COLOR_CYAN);
        int ret = wait_until(
            console, context, std::chrono::seconds(30),
            {{detector}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect trade warning after 30 seconds.");
        }
        console.log("Detected trade warning window.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }

    //  Start trade.
    pbf_press_button(context, BUTTON_A, 20, 0);

    //  Wait for black screen.
    {
        BlackScreenOverWatcher black_screen(COLOR_CYAN);
        int ret = wait_until(
            console, context, std::chrono::minutes(2),
            {{black_screen}}
        );
        if (ret < 0){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to detect start of trade after 2 minutes.");
        }
        console.log("Detected start of trade.");
        context.wait_for(std::chrono::milliseconds(100));
        tracker.check_unrecoverable_error(console);
    }


    TradeDoneHold trade_done(console);

    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW, std::chrono::seconds(2));
        PromptDialogWatcher learn_move(COLOR_BLUE);

        int ret = wait_until(
            console, context, std::chrono::minutes(2),
            {dialog, trade_done, learn_move}
        );
        switch (ret){
        case 0:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 1:
            console.log("Detected box. Trade completed.");
            tracker.check_unrecoverable_error(console);
            context.wait_for(std::chrono::milliseconds(500));
            return;
        case 2:
            console.log("Detected move learn.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            return;
        default:
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Failed to return to box after 2 minutes after a trade.");
        }
    }
}


void trade_current_box(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    EventNotificationOption& notifications,
    TradeStats& stats
){
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t col = 0; col < 6; col++){
            env.update_stats();
            send_program_status_notification(env, notifications);

            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
                BoxDetector box_detector;
                VideoOverlaySet overlays(console.overlay());
                box_detector.make_overlays(overlays);

                box_detector.move_cursor(console, context, BoxCursorLocation::SLOTS, row, col);

                trade_current_pokemon(console, context, error_state, stats);
            });
            stats.m_trades++;
        }
    }
}




}
}
}
