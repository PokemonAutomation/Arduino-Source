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
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/PokemonSV_Settings.h"
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
    TradeWarningDetector()
        : m_box_top(0.30, 0.33, 0.40, 0.02)
        , m_box_bot(0.30, 0.65, 0.40, 0.02)
    {}
    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(COLOR_RED, m_box_top);
        items.add(COLOR_RED, m_box_bot);
    }
    virtual bool detect(const ImageViewRGB32& screen) const override{
        ImageStats box_top = image_stats(extract_box_reference(screen, m_box_top));
        if (!is_solid(box_top, {0.11424, 0.310539, 0.575221}, 0.30)){
            return false;
        }
        ImageStats box_bot = image_stats(extract_box_reference(screen, m_box_bot));
        if (!is_solid(box_bot, {0.11424, 0.310539, 0.575221}, 0.30)){
            return false;
        }
        return true;
    }

protected:
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_bot;
};
class TradeWarningFinder : public DetectorToFinder_ConsecutiveDebounce<TradeWarningDetector>{
public:
    TradeWarningFinder()
         : DetectorToFinder_ConsecutiveDebounce("TradeWarningFinder", 5)
    {}
};



class TradeDoneDetector : public StaticScreenDetector{
public:
    TradeDoneDetector(VideoOverlay& overlay)
        : m_cursor(GradientArrowType::DOWN, {0.24, 0.17, 0.38, 0.55})
        , m_slot(true)
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
class TradeDoneHold : public DetectorToFinder_HoldDebounce<TradeDoneDetector>{
public:
    TradeDoneHold(VideoOverlay& overlay)
         : DetectorToFinder_HoldDebounce("AdvanceDialogHold", std::chrono::milliseconds(500), overlay)
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
        SomethingInBoxSlotDetector detector(true);
        if (!detector.detect(console.video().snapshot())){
            stats.m_errors++;
            tracker.report_unrecoverable_error(console, "Box slot is empty.");
        }
    }

    {
        pbf_press_button(context, BUTTON_A, 20, 0);
        context.wait_for_all_requests();
        GradientArrowFinder detector(console, GradientArrowType::RIGHT, {0.30, 0.18, 0.38, 0.08});
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
        PromptDialogFinder detector({0.500, 0.455, 0.400, 0.100}, COLOR_RED);
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
        TradeWarningFinder detector;
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
        BlackScreenOverWatcher black_screen;
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
        AdvanceDialogHold dialog(std::chrono::seconds(2));

        int ret = wait_until(
            console, context, std::chrono::minutes(2),
            {dialog, trade_done}
        );
        switch (ret){
        case 0:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 1:
            console.log("Detected box. Trade completed.");
            tracker.check_unrecoverable_error(console);
            context.wait_for(std::chrono::milliseconds(100));
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
    for (size_t row = 0; row < 5; row++){
        for (size_t col = 0; col < 6; col++){
            env.update_stats();
            send_program_status_notification(env, notifications);

            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
                trade_current_pokemon(console, context, error_state, stats);

                //  Move to next mon
                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);

                //  Move to next row, but only if not the last row.
                if (col == 5 && row < 4){
                    pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
                    pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                }

#if 0
                //  Move to next box.
                if (row == 5){
                    pbf_press_button(context, BUTTON_R, 20, 105);
                    pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
                    pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                    pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                    pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                }
#endif
            });
            stats.m_trades++;
        }
    }
}




}
}
}
