/*  Battle Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BattleSelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class BattleSelectionArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BattleSelectionArrowMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(35, 40, 40), Color(100, 100, 100), 70
        )
    {
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const BattleSelectionArrowMatcher& matcher(){
        static BattleSelectionArrowMatcher matcher("PokemonFRLG/BattleSelectionArrow.png");
        return matcher;
    }
};

ImageFloatBox BattleSelectionArrowDetector::box_for_option(BattleMenuOption option){
    switch (option){
    case BattleMenuOption::FIGHT:
        return ImageFloatBox(0.525, 0.765, 0.040, 0.080);
    case BattleMenuOption::BAG:
        return ImageFloatBox(0.760, 0.765, 0.040, 0.080);
    case BattleMenuOption::POKEMON:
        return ImageFloatBox(0.525, 0.865, 0.040, 0.080);
    case BattleMenuOption::RUN:
        return ImageFloatBox(0.760, 0.865, 0.040, 0.080);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Battle Menu Option");
}
ImageFloatBox BattleSelectionArrowDetector::box_for_option(SafariBattleMenuOption option){
    switch (option){
    case SafariBattleMenuOption::BALL:
        return ImageFloatBox(0.525, 0.765, 0.040, 0.080);
    case SafariBattleMenuOption::BAIT:
        return ImageFloatBox(0.760, 0.765, 0.040, 0.080);
    case SafariBattleMenuOption::ROCK:
        return ImageFloatBox(0.525, 0.865, 0.040, 0.080);
    case SafariBattleMenuOption::RUN:
        return ImageFloatBox(0.760, 0.865, 0.040, 0.080);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Safari Battle Menu Option");
}
ImageFloatBox BattleSelectionArrowDetector::box_for_option(BattleConfirmationOption option){
    switch (option){
    case BattleConfirmationOption::YES:
        return ImageFloatBox(0.795, 0.465, 0.030, 0.071);
    case BattleConfirmationOption::NO:
        return ImageFloatBox(0.795, 0.569, 0.030, 0.071);
    default:
        break;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG Battle Confirmation Option");
}

BattleSelectionArrowDetector::BattleSelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
BattleSelectionArrowDetector::BattleSelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    BattleMenuOption option
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box_for_option(option))
{}
BattleSelectionArrowDetector::BattleSelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    SafariBattleMenuOption option
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box_for_option(option))
{}
BattleSelectionArrowDetector::BattleSelectionArrowDetector(
    Color color,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(ImageFloatBox(0.768, 0.868, 0.212, 0.159))
{
}
BattleSelectionArrowDetector::BattleSelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    BattleConfirmationOption option
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box_for_option(option))
{}
void BattleSelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_arrow_box));
}
bool BattleSelectionArrowDetector::detect(const ImageViewRGB32& screen){
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);

    double screen_rel_size = (game_screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff000000, 0xff7f7f7f},
        {0xff232828, 0xff646464},
    };

    bool found = match_template_by_waterfill(
        game_screen.size(),
        extract_box_reference(game_screen, m_arrow_box),
        BattleSelectionArrowMatcher::matcher(),
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
