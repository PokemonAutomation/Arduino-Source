/*  Experience Gain Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_ExperienceGainDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ExperienceGainDetector::~ExperienceGainDetector(){}
ExperienceGainDetector::ExperienceGainDetector(Color color)
    : m_color(color)
    , m_dialog(color)
    , m_middle_column(0.369, 0.01, 0.003, 0.79)
    , m_left_column(0.006, 0.01, 0.03, 0.79)
    , m_lower_left_region(0.006, 0.8, 0.16, 0.15)
{}

void ExperienceGainDetector::make_overlays(VideoOverlaySet& items) const{
    m_dialog.make_overlays(items);
    items.add(m_color, m_middle_column);
    items.add(m_color, m_left_column);
    items.add(m_color, m_lower_left_region);
}
bool ExperienceGainDetector::detect(const ImageViewRGB32& screen){
    if (!m_dialog.detect(screen)){
//        cout << "ExperienceGainDetector: No dialogue detected, return" << endl;
        return false;
    }

    const ImageStats stats0 = image_stats(extract_box_reference(screen, m_middle_column));
//    cout << "ExperienceGainDetector: No m_middle_column detected, " << stats0.average.to_string() << ", " << stats0.stddev.to_string() << endl;
    if (!is_solid(stats0, {0.16, 0.42, 0.42}, 0.1, 40)){
        return false;
    }
    const ImageStats stats1 = image_stats(extract_box_reference(screen, m_left_column));
//    cout << "ExperienceGainDetector: No m_left_column detected, " << stats1.average.to_string() << ", " << stats1.stddev.to_string() << endl;
    if (!is_solid(stats1, {0.3, 0.35, 0.35}, 0.1, 40)){
        return false;
    }
    const ImageStats stats2 = image_stats(extract_box_reference(screen, m_lower_left_region));
//    cout << "ExperienceGainDetector: No m_lower_left_region detected, " << stats2.average.to_string() << ", " << stats2.stddev.to_string() << endl;
    if (!is_solid(stats2, {0.3, 0.35, 0.35}, 0.1, 40)){
        return false;
    }

//    std::cout << "ExperienceGainDetector: detect!" << std::endl;
    return true;
}


ExperienceGainWatcher::ExperienceGainWatcher(Color color)
     : ExperienceGainDetector(color)
     , VisualInferenceCallback("ExperienceGainWatcher")
{}
void ExperienceGainWatcher::make_overlays(VideoOverlaySet& items) const{
    ExperienceGainDetector::make_overlays(items);
}
bool ExperienceGainWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}




}
}
}
