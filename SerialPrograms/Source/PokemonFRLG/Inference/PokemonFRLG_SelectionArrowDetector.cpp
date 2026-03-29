/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ImageFloatBox SelectionArrowDetector::arrow_box_for_position(SelectionArrowPositionStartMenu position){
    switch (position){
    case SelectionArrowPositionStartMenu::POKEDEX:
        return ImageFloatBox(0.727692, 0.0523077, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::POKEMON:
        return ImageFloatBox(0.727692, 0.1457692, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::BAG:
        return ImageFloatBox(0.727692, 0.2392307, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::TRAINER:
        return ImageFloatBox(0.727692, 0.3378846, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::SAVE:
        return ImageFloatBox(0.727692, 0.4261538, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::OPTION:
        return ImageFloatBox(0.727692, 0.5248076, 0.0369231, 0.0778846);
    case SelectionArrowPositionStartMenu::EXIT:
        return ImageFloatBox(0.727692, 0.6182692, 0.0369231, 0.0778846);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Selection Arrow Position");
}

ImageFloatBox SelectionArrowDetector::arrow_box_for_position(SelectionArrowPositionSafariMenu position){
    // Safari Zone menu has the same 7 slots as the overworld menu; RETIRE occupies slot 0,
    // shifting POKEDEX..EXIT each down one relative to the overworld enum.
    switch (position){
    case SelectionArrowPositionSafariMenu::RETIRE:
        return ImageFloatBox(0.727692, 0.0523077, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::POKEDEX:
        return ImageFloatBox(0.727692, 0.1457692, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::POKEMON:
        return ImageFloatBox(0.727692, 0.2392307, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::BAG:
        return ImageFloatBox(0.727692, 0.3378846, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::TRAINER:
        return ImageFloatBox(0.727692, 0.4261538, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::OPTION:
        return ImageFloatBox(0.727692, 0.5248076, 0.0369231, 0.0778846);
    case SelectionArrowPositionSafariMenu::EXIT:
        return ImageFloatBox(0.727692, 0.6182692, 0.0369231, 0.0778846);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Safari Selection Arrow Position");
}

ImageFloatBox SelectionArrowDetector::arrow_box_for_position(SelectionArrowPositionConfirmationMenu position){
    switch (position){
    case SelectionArrowPositionConfirmationMenu::YES:
        return ImageFloatBox(0.69692, 0.4625, 0.037, 0.07788);
    case SelectionArrowPositionConfirmationMenu::NO:
        return ImageFloatBox(0.69692, 0.55, 0.037, 0.07788);
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
    SelectionArrowPositionStartMenu position
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(arrow_box_for_position(position))
{}
SelectionArrowDetector::SelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    SelectionArrowPositionConfirmationMenu position
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(arrow_box_for_position(position))
{
}
SelectionArrowDetector::SelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    SelectionArrowPositionSafariMenu position
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(arrow_box_for_position(position))
{
}
void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_arrow_box));
}
bool SelectionArrowDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);

    double screen_rel_size = (game_screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff464646, 0xff787878}
    };

    bool found = match_template_by_waterfill(
        game_screen.size(),
        extract_box_reference(game_screen, m_arrow_box),
        SelectionArrowMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(game_screen, m_arrow_box, object);
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