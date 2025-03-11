/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA_MapDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



MapDetector::MapDetector()
    : VisualInferenceCallback("MapDetector")
    , m_bottom0(0.450, 0.935, 0.500, 0.015)
    , m_bottom1(0.450, 0.965, 0.100, 0.020)
{}

void MapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_bottom0);
    items.add(COLOR_RED, m_bottom1);
}

//  Return true if the inference session should stop.
bool MapDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    ImageStats bottom0 = image_stats(extract_box_reference(frame, m_bottom0));
//    cout << bottom0.average << bottom0.stddev << endl;
    if (!is_solid(bottom0, {0.330212, 0.334083, 0.335705})){
        return false;
    }
    ImageStats bottom1 = image_stats(extract_box_reference(frame, m_bottom1));
//    cout << bottom1.average << bottom1.stddev << endl;
    if (!is_solid(bottom1, {0.32716, 0.33642, 0.33642})){
        return false;
    }

    if (bottom0.average.sum() < bottom1.average.sum()){
        return false;
    }

    return true;
}



}
}
}
