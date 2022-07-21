/*  Y-Comm Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSwSh_YCommDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


YCommMenuDetector::YCommMenuDetector(bool is_on)
    : VisualInferenceCallback("YCommMenuDetector")
    , m_is_on(is_on)
    , m_top(0.600, 0.020, 0.100, 0.040)
    , m_bottom(0.100, 0.970, 0.400, 0.020)
{}
void YCommMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_top);
    items.add(COLOR_RED, m_bottom);
}

bool YCommMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;
    if (!is_black(bottom)){
        return false;
    }

    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.0819777, 0.124031, 0.793991}, 0.2)){
        return false;
    }

    return true;
}

bool YCommMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_is_on ? detect(frame) : !detect(frame);
}


}
}
}
