/*  Bag Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV_BagDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



BagDetector::BagDetector(Color color)
    : m_color(color)
    , m_top_blue_left(0.24, 0.09, 0.05, 0.05)
    , m_top_blue_right(0.71, 0.09, 0.05, 0.05)
    , m_bottom(0.03, 0.94, 0.40, 0.04)
{}
void BagDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top_blue_left);
    items.add(m_color, m_top_blue_right);
    items.add(m_color, m_bottom);
}
bool BagDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_blue_left = image_stats(extract_box_reference(screen, m_top_blue_left));
//    cout << top_blue_left.average << top_blue_left.stddev << endl;
    if (!is_solid(top_blue_left, {0.0745162, 0.311321, 0.614163}, 0.30, 10)){
        return false;
    }

    ImageStats top_blue_right = image_stats(extract_box_reference(screen, m_top_blue_right));
//    cout << top_blue_right.average << top_blue_right.stddev << endl;
    if (!is_solid(top_blue_right, {0.0745162, 0.311321, 0.614163}, 0.30, 10)){
        return false;
    }

    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;
#if 0
    if (bottom.stddev.sum() > 20){
        return false;
    }
#else
    if (!is_summary_color(bottom)){
        return false;
    }
#endif

//    ImageStats shiny_symbol = image_stats(extract_box_reference(screen, m_shiny_symbol));
//    cout << shiny_symbol.average << shiny_symbol.stddev << endl;

    return true;
}





}
}
}
