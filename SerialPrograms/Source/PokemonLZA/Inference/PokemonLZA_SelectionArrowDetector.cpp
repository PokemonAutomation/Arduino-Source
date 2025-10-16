/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class SelectionArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // The white background for the template file is of color range [r=240, g=255, b=230] to [255, 255, 255]
    // the black arrow color is about [r=36,g=38,b=51]
    SelectionArrowMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(0xff80c000), Color(0xffffff7f), 100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const SelectionArrowMatcher& matcher(SelectionArrowType type){
        switch (type){
        case SelectionArrowType::RIGHT:
            return RIGHT_ARROW();
        case SelectionArrowType::DOWN:
            return DOWN_ARROW();
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid enum.");
        }
    }
    static const SelectionArrowMatcher& RIGHT_ARROW(){
        static SelectionArrowMatcher matcher("PokemonLZA/SelectionArrowRight.png");
        return matcher;
    }
    static const SelectionArrowMatcher& DOWN_ARROW(){
        static SelectionArrowMatcher matcher("PokemonLZA/SelectionArrowDown.png");
        return matcher;
    }
};



SelectionArrowDetector::SelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    SelectionArrowType type,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_type(type)
    , m_arrow_box(box)
{}
void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool SelectionArrowDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700.0;
    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff80c000, 0xffffff7f},
        {0xff80c000, 0xffdfff7f},
    };

    bool found = match_template_by_waterfill(
        extract_box_reference(screen, m_arrow_box),
        SelectionArrowMatcher::matcher(m_type),
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
