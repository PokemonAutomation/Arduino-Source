/*  Party Held Item Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PartyHeldItemDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class PartyHeldItemMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    PartyHeldItemMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(210, 210, 100),
            Color(255, 255, 190),
            20
        )
    {
        m_aspect_ratio_lower = 0.5;
        m_aspect_ratio_upper = 2.0;
        m_area_ratio_lower = 0.2;
        m_area_ratio_upper = 4.0;
    }

    static const PartyHeldItemMatcher& matcher(){
        static PartyHeldItemMatcher matcher("PokemonFRLG/PartyHeldItem.png");
        return matcher;
    }
};

ImageFloatBox PartyHeldItemDetector::box_for_slot(PartyHeldItemSlot slot){
    switch (slot){
    case PartyHeldItemSlot::SLOT_1:
        return ImageFloatBox(0.069, 0.285, 0.025, 0.050);
    case PartyHeldItemSlot::SLOT_2:
        return ImageFloatBox(0.432, 0.150, 0.025, 0.050);
    case PartyHeldItemSlot::SLOT_3:
        return ImageFloatBox(0.432, 0.3, 0.025, 0.050);
    case PartyHeldItemSlot::SLOT_4:
        return ImageFloatBox(0.432, 0.45, 0.025, 0.050);
    case PartyHeldItemSlot::SLOT_5 :
        return ImageFloatBox(0.432, 0.6, 0.025, 0.050);
    case PartyHeldItemSlot::SLOT_6:
        return ImageFloatBox(0.432, 0.725, 0.025, 0.050);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Party Held Item Slot");
}

PartyHeldItemDetector::PartyHeldItemDetector(
    Color color,
    VideoOverlay* overlay,
    PartyHeldItemSlot slot
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box_for_slot(slot))
{}
PartyHeldItemDetector::PartyHeldItemDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}
void PartyHeldItemDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}
bool PartyHeldItemDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);

    double screen_rel_size = (game_screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 100;
    double rmsd_threshold = 220;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd2d264, 0xffffffbe}
    };

    bool found = match_template_by_waterfill(
        game_screen.size(),
        extract_box_reference(game_screen, m_box),
        PartyHeldItemMatcher::matcher(),
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
