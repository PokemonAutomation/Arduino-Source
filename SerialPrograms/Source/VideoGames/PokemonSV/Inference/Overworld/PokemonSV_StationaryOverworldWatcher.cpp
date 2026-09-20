/*  Stationary Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonSV_StationaryOverworldWatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





StationaryOverworldWatcher::StationaryOverworldWatcher(Color color, ImageFloatBox box, size_t seconds_stationary)
     : VisualInferenceCallback("StationaryOverworldWatcher")
     , m_color(color)
     , m_box(box)
     , m_overworld_detector(color)
     , m_map_hold_duration(std::chrono::milliseconds(seconds_stationary * 1000))
{}

void StationaryOverworldWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool StationaryOverworldWatcher::process_frame(const VideoSnapshot& frame){
    if (!m_overworld_detector.detect(frame)){
        m_snapshot_start.clear();
        return false;
    }

    if (!m_snapshot_start){
        m_snapshot_start = frame;
        return false;
    }

    // Check if radar map stays still for `m_map_hold_duration`

    //  Mismatching image sizes.
    ImageViewRGB32 start = extract_box_reference(m_snapshot_start, m_box);
    ImageViewRGB32 current = extract_box_reference(frame, m_box);
    if (start.width() != current.width() || start.height() != current.height()){
        m_snapshot_start = frame;
        return false;
    }
    
    double rmsd = ImageMatch::pixel_RMSD(start, current);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd > 2.0){ //  Image of radar map has changed too much.
        m_snapshot_start = frame;
        return false;
    }

    //  Make sure radar map held for long enough.
    return frame.timestamp - m_snapshot_start.timestamp >= m_map_hold_duration;    
}



}
}
}
