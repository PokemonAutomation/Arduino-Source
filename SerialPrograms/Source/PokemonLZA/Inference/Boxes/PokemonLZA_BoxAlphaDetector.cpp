/*  Box Alpha Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA_BoxAlphaDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class BoxAlphaSymbolMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BoxAlphaSymbolMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(combine_rgb(200, 50, 50)), Color(combine_rgb(255, 100, 100)), 200
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const BoxAlphaSymbolMatcher& matcher(){
        const static BoxAlphaSymbolMatcher matcher("PokemonLZA/AlphaSymbol-Template.png");
        return matcher;
    }
};

BoxAlphaDetector::BoxAlphaDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_box{0.820, 0.107, 0.085, 0.042}
    , m_overlay(overlay)
{}

void BoxAlphaDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxAlphaDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 300.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {combine_rgb(200, 50, 50), combine_rgb(255, 100, 100)},
        {combine_rgb(150, 0, 0), combine_rgb(255, 100, 100)},
    };

    const double max_rsmd = 100.0;
    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        BoxAlphaSymbolMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        max_rsmd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
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
