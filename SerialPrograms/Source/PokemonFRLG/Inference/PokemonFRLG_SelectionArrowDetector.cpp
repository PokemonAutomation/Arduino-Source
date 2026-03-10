/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ImageFloatBox SelectionArrowDetector::arrow_box_for_position(SelectionArrowPosition position){
    switch (position){
    case SelectionArrowPosition::START_MENU_POKEDEX:
        return ImageFloatBox(0.685, 0.055, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_POKEMON:
        return ImageFloatBox(0.685, 0.145, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_BAG:
        return ImageFloatBox(0.685, 0.235, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_TRAINER:
        return ImageFloatBox(0.685, 0.330, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_SAVE:
        return ImageFloatBox(0.685, 0.415, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_OPTION:
        return ImageFloatBox(0.685, 0.510, 0.03, 0.075);
    case SelectionArrowPosition::START_MENU_EXIT:
        return ImageFloatBox(0.685, 0.6, 0.03, 0.075);
    case SelectionArrowPosition::CHOICE_MENU_YES:
        return ImageFloatBox(0.660, 0.450, 0.03, 0.075);
    case SelectionArrowPosition::CHOICE_MENU_NO:
        return ImageFloatBox(0.660, 0.535, 0.03, 0.075);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Selection Arrow Position");
}

class SelectionArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SelectionArrowMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(50, 50, 50), Color(115, 115, 115), 70
        )
    {
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const SelectionArrowMatcher& matcher(){
        static SelectionArrowMatcher matcher("PokemonFRLG/SelectionArrow.png");
        return matcher;
    }
};

SelectionArrowDetector::SelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
SelectionArrowDetector::SelectionArrowDetector(
    Color color, 
    VideoOverlay* overlay, 
    SelectionArrowPosition position
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(arrow_box_for_position(position))
{}
void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool SelectionArrowDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff464646, 0xff787878}
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        SelectionArrowMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
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