/*  Sandwich Ingredient Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/OCR/OCR_Routines.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV/Resources/PokemonSV_SandwichSprites.h"
#include "PokemonSV_SandwichIngredientDetector.h"

// #include <iostream>
//using std::cout;
//using std::endl;

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
        if (avg.r > avg.b * 1.25 && avg.g > avg.b * 1.15){
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

ImageMatch::SilhouetteDictionaryMatcher make_SANDWICH_FILLING_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (const auto& item : ALL_SANDWICH_FILLINGS()){
        matcher.add(item.first, item.second.icon.copy());
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& SANDWICH_FILLING_MATCHER() {
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_SANDWICH_FILLING_MATCHER();
    return matcher;
}

ImageMatch::SilhouetteDictionaryMatcher make_SANDWICH_CONDIMENT_MATCHER() {
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (const auto& item : ALL_SANDWICH_CONDIMENTS()) {
        matcher.add(item.first, item.second.icon.copy());
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& SANDWICH_CONDIMENT_MATCHER() {
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_SANDWICH_CONDIMENT_MATCHER();
    return matcher;
}

const SandwichFillingOCR& SandwichFillingOCR::instance(){
    static SandwichFillingOCR reader;
    return reader;
}

SandwichFillingOCR::SandwichFillingOCR()
    : SmallDictionaryMatcher("PokemonSV/Picnic/SandwichFillingOCR.json")
{}


OCR::StringMatchResult SandwichFillingOCR::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}

const SandwichCondimentOCR& SandwichCondimentOCR::instance() {
    static SandwichCondimentOCR reader;
    return reader;
}

SandwichCondimentOCR::SandwichCondimentOCR()
    : SmallDictionaryMatcher("PokemonSV/Picnic/SandwichCondimentOCR.json")
{}


OCR::StringMatchResult SandwichCondimentOCR::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}

SandwichIngredientReader::SandwichIngredientReader(SandwichIngredientType ingredient_type, size_t index, Color color)
    : m_color(color)
    , m_icon_box(0.061, 0.179 + 0.074 * index, 0.032, 0.057)
    , m_text_box(0.100, 0.179 + 0.074 * index, 0.273, 0.057)
    , m_ingredient_type(ingredient_type)
{}

void SandwichIngredientReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_icon_box);
    items.add(m_color, m_text_box);
}

SandwichIngredientReader::Results SandwichIngredientReader::read(const ImageViewRGB32& screen, Logger& logger,Language language) const{
    static constexpr double MAX_ALPHA = 100;
    static constexpr double ALPHA_SPREAD = 20;

    // Get a crop of the sandwich ingredient icon
    ImageViewRGB32 icon_image = extract_box_reference(screen, m_icon_box);
    //cropped_image.save("icon_image.png");

    // Get a crop of the sandwich ingredient text
    ImageViewRGB32 text_image = extract_box_reference(screen, m_text_box);
    //cropped_image.save("text_image.png");

    Results results;
    switch (m_ingredient_type){
    case SandwichIngredientType::FILLING:
        results.image_results = SANDWICH_FILLING_MATCHER().match(icon_image, ALPHA_SPREAD);
        results.ocr_results = SandwichFillingOCR::instance().read_substring(logger, language, text_image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());
        break;
    case SandwichIngredientType::CONDIMENT:
        results.image_results = SANDWICH_CONDIMENT_MATCHER().match(icon_image, ALPHA_SPREAD);
        results.ocr_results = SandwichCondimentOCR::instance().read_substring(logger, language, text_image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());
        break;
    }
    results.image_results.clear_beyond_alpha(MAX_ALPHA);

    return results;
}

}
}
}
