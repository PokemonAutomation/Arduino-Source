/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <array>

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_DialogBoxDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


namespace{
    // This box covers all possible locations of the black triangle arrow
    std::array<ImageFloatBox, 4> BLACK_BOXES{{
        {0.254, 0.806, 0.012, 0.052},
        {0.730, 0.806, 0.016, 0.068},
        {0.741, 0.880, 0.005, 0.055},
        {0.710, 0.880, 0.007, 0.055},
    }};
}


BlackDialogBoxDetector::BlackDialogBoxDetector(
    bool stop_on_detected
)
    : VisualInferenceCallback("BlackDialogBoxDetector")
    , m_stop_on_detected(stop_on_detected)
{}


void BlackDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    for(const auto& box : BLACK_BOXES){
        items.add(COLOR_RED, box);
    }
}


bool BlackDialogBoxDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool detected = true;
    for (const auto& box : BLACK_BOXES){
        if (is_black(extract_box_reference(frame, box), 200, 35) == false){
            detected = false;
            break;
        }
    }

    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}





}
}
}

