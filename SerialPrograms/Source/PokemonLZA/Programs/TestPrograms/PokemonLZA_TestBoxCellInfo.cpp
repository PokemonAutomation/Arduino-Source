/*  Test Box Cell Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA_TestBoxCellInfo.h"

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


TestBoxCellInfo_Descriptor::TestBoxCellInfo_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:TestBoxCellInfo",
        STRING_POKEMON + " LZA", "Test Box Cell Info",
        "",
        "Test program to check inference on pokemon info in box system. "
        "Move around on box cells while running the pokemon. Log overlay will show current selected cell and pokemon info of the cell.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


TestBoxCellInfo::TestBoxCellInfo() {}


// A watcher that keeps shows current box cell info
class BoxInfoWatcher : public VisualInferenceCallback{
public:
    BoxInfoWatcher(SingleSwitchProgramEnvironment& env, Color color = COLOR_RED, VideoOverlay* overlay = nullptr)
    : VisualInferenceCallback("BoxInfoWatcher")
    , m_box_detector(color, overlay)
    , m_sth_in_cell_detector(color, overlay)
    , m_shiny_detector(color, overlay)
    , m_alpha_detector(color, overlay)
    , m_env(env)
    {}

    virtual ~BoxInfoWatcher() {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        m_box_detector.make_overlays(items);
        m_sth_in_cell_detector.make_overlays(items);
        m_shiny_detector.make_overlays(items);
        m_alpha_detector.make_overlays(items);
    }
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        bool box_cursor_found = m_box_detector.process_frame(frame, timestamp);
        if (!box_cursor_found){
            return false; // not in box view
        }

        auto coord = m_box_detector.detected_location();
        if (coord.row != m_last_row || coord.col != m_last_col){
            // cout << "selected cell location moved: (" << int(m_last_row) << ", " << int(m_last_col)
            //      << ") -> (" << int(coord.row) << ", " << int(coord.col) << ")"
            //      << " need to update log" << endl;
            m_last_row = coord.row;
            m_last_col = coord.col;

            // we are in a new cell, reset detectors. Prepare to update log once determined info on the new cell
            m_sth_in_cell_detector.reset_state();
            m_alpha_detector.reset_state();
            m_shiny_detector.reset_state();
            m_need_to_update_log_once_determined = true;
        }

        bool sth_in_cell_determined = m_sth_in_cell_detector.process_frame(frame, timestamp);
        bool is_shiny_determined = m_shiny_detector.process_frame(frame, timestamp);
        bool is_alpha_determined = m_alpha_detector.process_frame(frame, timestamp);

        

        // detector is not determined, this means sth may have changed, we need to update the log
        if (!sth_in_cell_determined || !is_shiny_determined || !is_alpha_determined){
            // cout << "sth not determined! need to update log" << endl;
            // cout << "box cursor found? " << box_cursor_found << " sth in cell determined? " << sth_in_cell_determined
            //      << " shiny determined? " << is_shiny_determined << " alpha determined? " << is_alpha_determined << endl;
            m_need_to_update_log_once_determined = true;
        }

        if (m_need_to_update_log_once_determined && sth_in_cell_determined && is_shiny_determined && is_alpha_determined){
            bool sth_in_cell = m_sth_in_cell_detector.consistent_result();
            bool is_shiny = m_shiny_detector.consistent_result();
            bool is_alpha = m_alpha_detector.consistent_result();

            std::ostringstream os;
            os << "Cell (" << int(m_last_row) << ", " << int(m_last_col) << ") ";
            if (!sth_in_cell){
                os << "Empty";
            } else{
                if (is_shiny && is_alpha){
                    os << "Shiny Alpha Pokemon";
                }
                else if (is_shiny){
                    os << " Shiny Pokemon";
                } else if (is_alpha){
                    os << " Alpha Pokemon";
                } else {
                    os << " Normal Pokemon";
                }
            }
            m_env.console.overlay().add_log(os.str());
            m_need_to_update_log_once_determined = false;
        }
        // cout << "detection result: (" << int(coord.row) << ", " << int(coord.col) << ")"
        //      << " sth in cell? " << m_sth_in_cell_detector.consistent_result()
        //      << " shiny? " << m_shiny_detector.consistent_result()
        //      << " alpha? " << m_alpha_detector.consistent_result() << endl;

        return false;
    }


protected:
    uint8_t m_last_row = BoxCursorCoordinates::INVALID;
    uint8_t m_last_col = BoxCursorCoordinates::INVALID;
    BoxWatcher m_box_detector;
    SomethingInBoxCellWatcher m_sth_in_cell_detector;
    BoxShinyWatcher m_shiny_detector;
    BoxAlphaWatcher m_alpha_detector;
    SingleSwitchProgramEnvironment& m_env;
    bool m_need_to_update_log_once_determined = true;
};


void TestBoxCellInfo::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    env.log("Starting Test Box Cell Info test program...");

    BoxInfoWatcher watcher(env);

    wait_until(env.console, context, WallClock::max(), {watcher});
}




}
}
}
