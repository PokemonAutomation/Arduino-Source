/*  Sandwich Ingredient Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_SandwichIngredientDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class SandwichCondimentsPageMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichCondimentsPageMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/Condiments-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichCondimentsPageMatcher matcher;
        return matcher;
    }
};

class SandwichPicksPageMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichPicksPageMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/Picks-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
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

bool SandwichIngredientArrowDetector::detect(const ImageViewRGB32& screen){
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

bool DeterminedSandwichIngredientDetector::detect(const ImageViewRGB32& screen){
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

bool SandwichCondimentsPageDetector::detect(const ImageViewRGB32& screen){
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

bool SandwichPicksPageDetector::detect(const ImageViewRGB32& screen){
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




const SandwichFillingMatcher& SANDWICH_FILLING_MATCHER(){
    static SandwichFillingMatcher matcher;
    return matcher;
}
SandwichFillingMatcher::SandwichFillingMatcher(const std::vector<double>& min_euclidean_distance)
    : CroppedImageDictionaryMatcher({0, 1})
{
    for (double x : min_euclidean_distance){
        m_min_euclidean_distance_squared.emplace_back(x * x);
    }
    for (const auto& item : SANDWICH_FILLINGS_DATABASE()){
        add(item.first, item.second.sprite);
    }
}
auto SandwichFillingMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
//    cout << "border = " << border.average << endl;
//    image.save("image.png");
    std::vector<ImageViewRGB32> ret;
    for (double min_euclidean_distance_squared : m_min_euclidean_distance_squared){
        ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](Color pixel){
                double r = (double)pixel.red() - border.average.r;
                double g = (double)pixel.green() - border.average.g;
                double b = (double)pixel.blue() - border.average.b;
                bool stop = r * r + g * g + b * b >= min_euclidean_distance_squared;
                return stop;
            }
        );
        ret.emplace_back(extract_box_reference(image, box));
    }
    return ret;
}


const SandwichCondimentMatcher& SANDWICH_CONDIMENT_MATCHER(){
    static SandwichCondimentMatcher matcher;
    return matcher;
}
SandwichCondimentMatcher::SandwichCondimentMatcher(const std::vector<double>& min_euclidean_distance)
    : CroppedImageDictionaryMatcher({0, 1})
{
    for (double x : min_euclidean_distance){
        m_min_euclidean_distance_squared.emplace_back(x * x);
    }
    for (const auto& item : SANDWICH_CONDIMENTS_DATABASE()){
        add(item.first, item.second.sprite);
    }
}
auto SandwichCondimentMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
    std::vector<ImageViewRGB32> ret;
    for (double min_euclidean_distance_squared : m_min_euclidean_distance_squared){
        ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](Color pixel){
                double r = (double)pixel.red() - border.average.r;
                double g = (double)pixel.green() - border.average.g;
                double b = (double)pixel.blue() - border.average.b;
                bool stop = r * r + g * g + b * b >= min_euclidean_distance_squared;
                return stop;
            }
        );
        ret.emplace_back(extract_box_reference(image, box));
    }
    return ret;
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

const SandwichCondimentOCR& SandwichCondimentOCR::instance(){
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

SandwichIngredientReader::SandwichIngredientReader(SandwichIngredientType ingredient_type, Color color)
    : m_color(color)
    , m_ingredient_type(ingredient_type)
    , m_box_ingred_text(ingredient_list_boxes(ImageFloatBox(0.100, 0.179, 0.273, 0.057)))
    , m_box_ingred_icon(ingredient_list_boxes(ImageFloatBox(0.064, 0.179, 0.032, 0.057)))
    , m_box_confirmed(confirmed_ingredient_boxes(ingredient_type))
{}

void SandwichIngredientReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < INGREDIENT_PAGE_LINES; c++){
        items.add(m_color, m_box_ingred_text[c]);
        items.add(m_color, m_box_ingred_icon[c]);
    }

    for (size_t i = 0; i < m_box_confirmed.size(); i++){
        items.add(m_color, m_box_confirmed[i]);
    }
}


std::array<ImageFloatBox, 6> SandwichIngredientReader::confirmed_ingredient_boxes(SandwichIngredientType type){
    std::array<ImageFloatBox, 6> boxes;
    ImageFloatBox initial_box;
    size_t total_count = 0;
    switch (type){
    case SandwichIngredientType::FILLING:
        initial_box = ImageFloatBox(0.508781, 0.820, 0.032, 0.057);
        total_count = 6;
        break;
    case SandwichIngredientType::CONDIMENT:
        initial_box = ImageFloatBox(0.797474, 0.820, 0.032, 0.057);
        total_count = 4;
        break;
    }
    
    double initial_x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double y = initial_box.y;
    double x_spacing = 0.0468;
    for (size_t i = 0; i < total_count; i++){
        double x = initial_x + i*x_spacing;
        boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return boxes;
}


std::array<ImageFloatBox, 10> SandwichIngredientReader::ingredient_list_boxes(ImageFloatBox initial_box){
    std::array<ImageFloatBox, 10> material_boxes;
    double x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double initial_y = initial_box.y;
    double y_spacing = 0.074;
    for (size_t i = 0; i < 10; i++){
        double y = initial_y + i*y_spacing;
        material_boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return material_boxes;
}


ImageMatch::ImageMatchResult SandwichIngredientReader::read_ingredient_page_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const{
    return read_with_icon_matcher(screen, m_box_ingred_icon[index]);
}

ImageMatch::ImageMatchResult SandwichIngredientReader::read_confirmed_list_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const{
    return read_with_icon_matcher(screen, m_box_confirmed[index]);
}

ImageMatch::ImageMatchResult SandwichIngredientReader::read_with_icon_matcher(const ImageViewRGB32& screen, const ImageFloatBox icon_box) const{
    // Get a crop of the sandwich ingredient icon
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
//    image.save("image" + std::to_string(icon_box.x) + ".png");

//    // Remove the orange / yellow background when the ingredient is selected
//    ImageRGB32 filtered_image = filter_rgb32_range(image, 0xffdfaf00, 0xffffef20, Color(0x00000000), true);
//    filtered_image.save("filtered_image.png");

    ImageMatch::ImageMatchResult results;
    switch (m_ingredient_type){
    case SandwichIngredientType::FILLING:
//        cout << "Filling" << endl;
        results = SANDWICH_FILLING_MATCHER().match(image, ALPHA_SPREAD);
        break;
    case SandwichIngredientType::CONDIMENT:
//        cout << "Condiment" << endl;
        results = SANDWICH_CONDIMENT_MATCHER().match(image, ALPHA_SPREAD);
        break;
    }
//    results.clear_beyond_alpha(MAX_ALPHA);

    return results;
}

OCR::StringMatchResult SandwichIngredientReader::read_ingredient_page_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    size_t index
) const{
    return read_with_ocr(screen, logger, language, m_box_ingred_text[index]);
}

OCR::StringMatchResult SandwichIngredientReader::read_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    const ImageFloatBox icon_box
) const{

    // Get a crop of the sandwich ingredient text
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
    //image.save("image.png");

    OCR::StringMatchResult results;
    switch (m_ingredient_type){
    case SandwichIngredientType::FILLING:
        results = SandwichFillingOCR::instance().read_substring(logger, language, image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());
        break;
    case SandwichIngredientType::CONDIMENT:
        results = SandwichCondimentOCR::instance().read_substring(logger, language, image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());
        break;
    }

    return results;
}

}
}
}
