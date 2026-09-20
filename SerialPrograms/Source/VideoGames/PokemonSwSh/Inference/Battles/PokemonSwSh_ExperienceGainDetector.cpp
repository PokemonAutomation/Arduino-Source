/*  Experience Gain Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_ExperienceGainDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ExperienceGainDetector::~ExperienceGainDetector(){}
ExperienceGainDetector::ExperienceGainDetector(Color color)
    : m_color(color)
    , m_dialog(color)
    , m_rows(6)
{
    const double SHIFT_X = 0.0325;
    const double SHIFT_Y = 0.126;
    for (size_t c = 0; c < 6; c++){
        m_rows.emplace_back(
            std::piecewise_construct,
            std::forward_as_tuple(0.255            , 0.04 + c*SHIFT_Y, 0.18 - c*SHIFT_X, 0.08),
            std::forward_as_tuple(0.475 - c*SHIFT_X, 0.04 + c*SHIFT_Y, 0.05            , 0.08)
        );
    }
}

void ExperienceGainDetector::make_overlays(VideoOverlaySet& items) const{
    m_dialog.make_overlays(items);
    for (const auto& item : m_rows){
        items.add(m_color, item.first);
        items.add(m_color, item.second);
    }
}
bool ExperienceGainDetector::detect(const ImageViewRGB32& screen){
    if (!m_dialog.detect(screen)){
        return false;
    }

    for (auto& row : m_rows){
        ImageStats stats0 = image_stats(extract_box_reference(screen, row.first));
//        cout << stats0.average << " " << stats0.stddev << endl;
        if (!is_grey(stats0, 400, 1000)){
            return false;
        }
        ImageStats stats1 = image_stats(extract_box_reference(screen, row.second));
//        cout << stats1.average << " " << stats1.stddev << endl;
        if (!is_white(stats1)){
            return false;
        }
        if (stats0.average.sum() > stats1.average.sum()){
            return false;
        }
    }

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
