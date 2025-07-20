/*  Tutorial Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_TutorialDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


TutorialDetector::TutorialDetector(Color color)
    : m_color(color)
    , m_bottom(0.235, 0.87, 0.10, 0.035)
    , m_left(0.230, 0.18, 0.02, 0.72)
{}
void TutorialDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left);
}

// detect solid beige color along the side and bottom of the tutorial
bool TutorialDetector::detect(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
    ImageStats left = image_stats(extract_box_reference(screen, m_left));
    // cout << "bottom.average.sum(): " << bottom.average.sum() << endl;
    // cout << "left.average.sum(): " << left.average.sum() << endl;

    // expected color is {255, 255, 235}
    return is_solid(bottom, {0.342282, 0.342282, 0.315436}) 
            && bottom.average.sum() > 500
            && is_solid(left, {0.342282, 0.342282, 0.315436})
            && left.average.sum() > 500;
}



}
}
}
