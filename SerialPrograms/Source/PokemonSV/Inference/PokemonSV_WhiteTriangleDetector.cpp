/*  White Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV_WhiteTriangleDetector.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class WhiteTriangleMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    WhiteTriangleMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/WhiteTriangleIcon-Template.png", Color(200,200,200), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.4;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static WhiteTriangleMatcher matcher;
        return matcher;
    }
};


WhiteTriangleDetector::~WhiteTriangleDetector() = default;

WhiteTriangleDetector::WhiteTriangleDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}

void WhiteTriangleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}


bool WhiteTriangleDetector::detect(const ImageViewRGB32& screen) {
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(240, 240, 240), combine_rgb(255, 255, 255)},
        {combine_rgb(220, 220, 220), combine_rgb(240, 240, 240)},
        {combine_rgb(200, 200, 200), combine_rgb(220, 220, 220)},
        {combine_rgb(190, 190, 190), combine_rgb(210, 210, 210)},

    };

    const double rmsd_threshold = 50.0;

    const double min_object_size = 100.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    bool is_found = false;
    match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        WhiteTriangleMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            is_found = true;
            return true;
        }
    );

    return is_found;
}



// WhiteTriangleWatcher::~WhiteTriangleWatcher() = default;

// WhiteTriangleWatcher::WhiteTriangleWatcher(Color color, const ImageFloatBox& box)
//     : VisualInferenceCallback("WhiteTriangleWatcher")
//     , m_detector(color, box)
// {}

// void WhiteTriangleWatcher::make_overlays(VideoOverlaySet& items) const{
//     m_detector.make_overlays(items);
// }

// bool WhiteTriangleWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
//     return m_detector.detect(screen);
// }








}
}
}
