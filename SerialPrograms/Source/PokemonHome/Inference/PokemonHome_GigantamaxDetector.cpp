/*  Gigantamax Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonHome_GigantamaxDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GigantamaxMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    GigantamaxMatcher()
        : WaterfillTemplateMatcher(
            "PokemonHome/Home_Gigantamax.png",
            Color(196, 32, 88), Color(255, 220, 235),
            100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const GigantamaxMatcher& instance(){
        static GigantamaxMatcher matcher;
        return matcher;
    }
};


GigantamaxDetector::GigantamaxDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}
void GigantamaxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool GigantamaxDetector::detect(const ImageViewRGB32& screen){
    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const double rmsd_threshold = 80;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc42058, 0xffffdceb},
    };

    const bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        GigantamaxMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}


}
}
}
