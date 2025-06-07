/*  Console Type Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "NintendoSwitch_ConsoleTypeDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


ConsoleTypeDetector_Home::ConsoleTypeDetector_Home(Color color)
    : m_color(color)
    , m_bottom_line(0.10, 0.88, 0.80, 0.03)
{}

void ConsoleTypeDetector_Home::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_line);
}
ConsoleTypeDetection ConsoleTypeDetector_Home::detect(const ImageViewRGB32& screen) const{
    ImageStats stats = image_stats(extract_box_reference(screen, m_bottom_line));
//    cout << stats.stddev.sum() << endl;
    if (stats.stddev.sum() < 10){
        return ConsoleTypeDetection::Switch2_Unknown;
    }else{
        return ConsoleTypeDetection::Switch1;
    }
}




}
}
