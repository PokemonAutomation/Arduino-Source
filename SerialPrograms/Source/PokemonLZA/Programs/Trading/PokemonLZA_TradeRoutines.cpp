/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA_TradeRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




TradeStats::TradeStats()
    : trades(m_stats["Trades"])
    , skipped(m_stats["Skipped"])
    , errors(m_stats["Errors"])
{
    m_display_order.emplace_back("Trades");
    m_display_order.emplace_back("Skipped", HIDDEN_IF_ZERO);
    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
}



#if 0
class SelectedDetector : public StaticScreenDetector{
public:
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

    Selection
};
#endif





bool trade_current_pokemon(
    VideoStream& stream, ProControllerContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
){
    tracker.check_unrecoverable_error(stream.logger());
    context.wait_for_all_requests();

    //  Make sure there is something to trade.
    {
        SomethingInBoxCellDetector detector(COLOR_CYAN, &stream.overlay());
        if (!detector.detect(stream.video().snapshot())){
            stats.errors++;
            tracker.report_unrecoverable_error(stream, "Box slot is empty.");
        }
    }


    bool traded = false;

    while (true){
        BoxWatcher box(COLOR_RED, &stream.overlay());
        SelectionArrowWatcher selection(
            COLOR_GREEN,
            &stream.overlay(),
            SelectionArrowType::RIGHT,
            {0.052894, 0.315615, 0.246422, 0.522702}
        );
        SelectionArrowWatcher confirm(
            COLOR_BLUE,
            &stream.overlay(),
            SelectionArrowType::RIGHT,
            {0.586808, 0.568106, 0.401369, 0.231451}
        );
        BlueDialogWatcher blue_dialog(COLOR_YELLOW, &stream.overlay());

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            120s,
            {
                box,
                selection,
                confirm,
                blue_dialog,
            }
        );
        tracker.check_unrecoverable_error(stream.logger());
        context.wait_for(100ms);
        switch (ret){
        case 0:
            stream.log("Detected box!");
            if (traded){
                return true;
            }else{
                pbf_press_button(context, BUTTON_A, 160ms, 840ms);
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            }
            continue;

        case 1:
            stream.log("Detected selection.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 2:
            stream.log("Detected confirmation.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            traded = true;
            continue;

        case 3:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        default:
            stats.errors++;
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

            bool ok[2] = {false, false};

            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                VideoOverlaySet overlays(console.overlay());

                BoxDetector detector(COLOR_RED, &console.overlay());
                detector.make_overlays(overlays);
                detector.move_cursor(env.program_info(), console, context, row + 1, col);

                ok[console.index()] = trade_current_pokemon(console, context, error_state, stats);
            });
            if (ok[0] && ok[1]){
                stats.trades++;
            }else{
                stats.skipped++;
            }
        }
        start_col = 0;
    }
}







}
}
}
