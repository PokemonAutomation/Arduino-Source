/*  Sandwich Ingredient Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_SandwichIngredientDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace {

class SandwichCondimentsPageMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichCondimentsPageMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/Condiments-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ) {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static SandwichCondimentsPageMatcher matcher;
        return matcher;
    }
};

class SandwichPicksPageMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichPicksPageMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/Picks-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ) {
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static SandwichPicksPageMatcher matcher;
        return matcher;
    }
};


} // anonymous namespace


SandwichIngredientArrowDetector::SandwichIngredientArrowDetector(size_t menu_index, Color color)
    : m_arrow(color, GradientArrowType::RIGHT, {0.013, menu_index*0.074 + 0.167, 0.056, 0.084}) {}

void SandwichIngredientArrowDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow.make_overlays(items);
}

bool SandwichIngredientArrowDetector::detect(const ImageViewRGB32& screen) const{
    return m_arrow.detect(screen);
}


DeterminedSandwichIngredientDetector::DeterminedSandwichIngredientDetector(
    SandwichIngredientType ingredient_type, size_t index, Color color
) : m_color(color){
    float offset = (ingredient_type == SandwichIngredientType::FILLING ? 0.0f : 0.2885f) + index * 0.047f;
    m_edges[0] = ImageFloatBox(offset + 0.509, 0.807, 0.033, 0.012);
    m_edges[1] = ImageFloatBox(offset + 0.501, 0.821, 0.008, 0.057);
    m_edges[2] = ImageFloatBox(offset + 0.509, 0.879, 0.033, 0.012);
    m_edges[3] = ImageFloatBox(offset + 0.541, 0.821, 0.008, 0.057);
}

void DeterminedSandwichIngredientDetector::make_overlays(VideoOverlaySet& items) const{
    for(int i = 0; i < 4; i++){
        items.add(m_color, m_edges[i]);
    }
}

bool DeterminedSandwichIngredientDetector::detect(const ImageViewRGB32& screen) const{
    int yellow_count = 0;
    for(int i = 0; i < 4; i++){
        FloatPixel avg = image_stats(extract_box_reference(screen, m_edges[i])).average;
        if (avg.r > avg.b * 1.25 && avg.g > avg.b * 1.25){
            yellow_count++;
        }
    }
    return yellow_count >= 3;
}


SandwichCondimentsPageDetector::SandwichCondimentsPageDetector(Color color)
    : m_color(color), m_box(0.046, 0.100, 0.021, 0.052) {}

void SandwichCondimentsPageDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool SandwichCondimentsPageDetector::detect(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (screen.height() / 1080.0);

    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 700);
    return match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        SandwichCondimentsPageMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        70,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
}



SandwichPicksPageDetector::SandwichPicksPageDetector(Color color)
    : m_color(color), m_box(0.046, 0.100, 0.021, 0.052) {}

void SandwichPicksPageDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool SandwichPicksPageDetector::detect(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (screen.height() / 1080.0);

    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 300);
    return match_template_by_waterfill(
        extract_box_reference(screen, m_box), 
        SandwichPicksPageMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        70,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
}

}
}
}
