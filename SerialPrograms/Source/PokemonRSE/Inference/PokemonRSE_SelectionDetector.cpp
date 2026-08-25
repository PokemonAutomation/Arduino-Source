/*  Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_SelectionDetector.h"

#include <iostream>

//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


class SelectionArrowMatcher : public ImageMatch::WaterfillTemplateMatcher {
public:
    SelectionArrowMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(50, 50, 50), Color(115, 115, 115), 24
        )
    {
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }


    static const SelectionArrowMatcher& emerald_matcher() {
        static SelectionArrowMatcher matcher("PokemonRSE/SelectionArrowEmerald.png");
        return matcher;
    }
    static const SelectionArrowMatcher& ruby_sapphire_jpn_matcher() {
        static SelectionArrowMatcher matcher("PokemonRSE/SelectionArrowRubySapphireJpn.png");
        return matcher;
    }
    static const SelectionArrowMatcher& emerald_jpn_matcher() {
        static SelectionArrowMatcher matcher("PokemonRSE/SelectionArrowEmeraldJpn.png");
        return matcher;
    }
};

SelectionArrowDetector::SelectionArrowDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}
bool SelectionArrowDetector::detect(const ImageViewRGB32& screen) {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);
    ImageViewRGB32 image = extract_box_reference(game_screen, m_box);

    double screen_rel_height = game_screen.height() / 160.0;
    double screen_rel_width = game_screen.width() / 240.0;
    double screen_rel_size = screen_rel_height * screen_rel_width;

    const double MIN_AREA_240 = 10;
    const double RMSD_THRESHOLD = 80;
    size_t min_area = size_t(screen_rel_size * MIN_AREA_240);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff4f4f4f, 0xff777777}, // Emerald
        {0xff2f2f2f, 0xff5c5c5c}, // Emerald
        {0xff363636, 0xff5e5e5e} // Ruby/Sapphire
    };

    auto try_match = [&](const SelectionArrowMatcher& matcher) -> bool {
        return match_template_by_waterfill(
            game_screen.size(),
            image,
            matcher,
            FILTERS,
            { min_area, SIZE_MAX },
            RMSD_THRESHOLD,
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                //std::cout << "area: " << object.area << std::endl;
                return true;
            }
        );
    };

    return try_match(SelectionArrowMatcher::emerald_matcher())
        || try_match(SelectionArrowMatcher::emerald_jpn_matcher())
        || try_match(SelectionArrowMatcher::ruby_sapphire_jpn_matcher());
}


SelectionBorderDetector::SelectionBorderDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void SelectionBorderDetector::make_overlays(VideoOverlaySet& items) const {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box));
}
bool SelectionBorderDetector::detect(const ImageViewRGB32& screen) {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    ImageViewRGB32 game_screen = extract_box_reference(screen, GAME_BOX);
    ImageViewRGB32 image = extract_box_reference(game_screen, m_box);

    double screen_rel_height = game_screen.height() / 160.0;
    double screen_rel_width = game_screen.width() / 240.0;
    double screen_rel_size = screen_rel_height * screen_rel_width;

    const double MIN_AREA_240 = 5; // the left/right side of the border is 12-13 pixels high
    size_t min_area = size_t(screen_rel_size * MIN_AREA_240);

    size_t pixels_in_range;
    filter_rgb32_range(
        pixels_in_range,
        image,
        0xffc82c23,
        0xffff9a91,
        Color(0),
        false
    );

    return pixels_in_range > min_area;
}


SelectionSlotDetector::SelectionSlotDetector(Color color, const std::vector<SlotCandidate>& candidates) 
    : m_color(color)
{
    for (const auto& candidate : candidates) {
        double slot_y = candidate.y;
        double width = (candidate.indicator == SelectionIndicator::ARROW) ? SELECTION_ARROW_BOX_WIDTH : SELECTION_BORDER_BOX_WIDTH;
        double height = (candidate.indicator == SelectionIndicator::BORDER) ? SELECTION_BORDER_BOX_HEIGHT : SELECTION_ARROW_BOX_HEIGHT;
        ImageFloatBox box(candidate.x, slot_y, width, height);

        if (candidate.indicator == SelectionIndicator::ARROW) {
            m_detectors.emplace_back(std::make_unique<SelectionArrowDetector>(color, box));
        } else if (candidate.indicator == SelectionIndicator::BORDER) {
            m_detectors.emplace_back(std::make_unique<SelectionBorderDetector>(color, box));
        }
    }
}
void SelectionSlotDetector::make_overlays(VideoOverlaySet& items) const {
    for (const auto& detector : m_detectors) {
        detector->make_overlays(items);
    }
}
bool SelectionSlotDetector::detect(const ImageViewRGB32& screen) {
    for (auto& detector : m_detectors) {
        if (detector->detect(screen)) {
            return true;
        }
    }
    return false;
}



}
}
}
