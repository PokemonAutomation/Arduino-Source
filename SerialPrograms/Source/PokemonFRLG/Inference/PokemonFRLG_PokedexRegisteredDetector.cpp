/*  Pokedex Registered Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PokedexRegisteredDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class PokedexRegisteredMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    PokedexRegisteredMatcher()
        : WaterfillTemplateMatcher(
            "PokemonFRLG/PokedexButtonA.png",
            Color(128, 128, 128), Color(255, 255, 255), 100
        )
    {
        m_aspect_ratio_lower = 0.5;
        m_aspect_ratio_upper = 2.0;
        m_area_ratio_lower = 0.5;
        m_area_ratio_upper = 2.0;
    }

    static const PokedexRegisteredMatcher& matcher(){
        static PokedexRegisteredMatcher matcher;
        return matcher;
    }
};


PokedexRegisteredDetector::PokedexRegisteredDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}

void PokedexRegisteredDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}

bool PokedexRegisteredDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);

    double screen_rel_size = (game_screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 70;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
    };

    bool found = match_template_by_waterfill(
        game_screen.size(),
        extract_box_reference(game_screen, m_box),
        PokedexRegisteredMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(game_screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, GAME_BOX.inner_to_outer(m_last_detected), COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}


}
}
}
