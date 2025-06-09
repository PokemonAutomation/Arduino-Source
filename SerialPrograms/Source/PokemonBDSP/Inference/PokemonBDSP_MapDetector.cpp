/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_MapDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



MapDetector::MapDetector(Color color)
    : m_color(color)
    , m_box0(0.68, 0.08, 0.06, 0.05)
    , m_box1(0.02, 0.97, 0.12, 0.02)
    , m_box2(0.88, 0.84, 0.10, 0.04)
{}
void MapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box0);
    items.add(m_color, m_box1);
    items.add(m_color, m_box2);
}
bool MapDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_box0));
//    cout << "m_box0: " << stats0.average << stats0.stddev << endl;
    if (!is_solid(stats0, {0.0668203, 0.4447, 0.488479})){
        return false;
    }
    ImageStats stats1 = image_stats(extract_box_reference(screen, m_box1));
//    cout << "m_box1: " << stats1.average << stats1.stddev << endl;
    if (!is_solid(stats1, {0.190189, 0.32745, 0.482361})){
        return false;
    }
    ImageStats stats2 = image_stats(extract_box_reference(screen, m_box2));
//    cout << "m_box2: " << stats2.average << stats2.stddev << endl;
    if (!is_solid(stats2, {0.0668203, 0.4447, 0.488479})){
        return false;
    }
    return true;
}




MapWatcher::MapWatcher(Color color)
    : MapDetector(color)
    , VisualInferenceCallback("MapWatcher")
{}
void MapWatcher::make_overlays(VideoOverlaySet& items) const{
    MapDetector::make_overlays(items);
}
bool MapWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}



}
}
}
