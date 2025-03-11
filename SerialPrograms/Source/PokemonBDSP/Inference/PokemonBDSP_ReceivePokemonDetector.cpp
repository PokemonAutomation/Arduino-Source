/*  Receive Pokemon (Blue Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_ReceivePokemonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



ReceivePokemonDetector::ReceivePokemonDetector(Color color)
    : VisualInferenceCallback("ReceivePokemonDetector")
    , m_color(color)
    , m_box0(0.05, 0.10, 0.10, 0.80)
    , m_box1(0.87, 0.10, 0.10, 0.80)
{}


void ReceivePokemonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box0);
    items.add(m_color, m_box1);
}

bool ReceivePokemonDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const ImageStats stats0 = image_stats(extract_box_reference(frame, m_box0));
    if (stats0.average.sum() < 100 || !is_solid(stats0, {0.22951, 0.340853, 0.429638}, 0.15, 20)){
        return m_received;
    }
    const ImageStats stats1 = image_stats(extract_box_reference(frame, m_box1));
    if (stats1.average.sum() < 100 || !is_solid(stats1, {0.22951, 0.340853, 0.429638}, 0.15, 20)){
        return m_received;
    }
    m_received = true;
//    static int c = 0;
//    frame.save("test-" + std::to_string(c++) + ".png");
    return false;
}


}
}
}
