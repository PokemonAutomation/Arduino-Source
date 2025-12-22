/*  Donut Berries Detector
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
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerriesDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


DonutBerriesSelectionDetector::DonutBerriesSelectionDetector(size_t menu_index, Color color)
    : m_symbol_box({0.001, menu_index*0.087 + 0.245, 0.006, 0.070}) {}

void DonutBerriesSelectionDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_symbol_box);
}

bool DonutBerriesSelectionDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 symbol = extract_box_reference(screen, m_symbol_box);
    if (is_solid(symbol, { 0.4375, 0.268, 0.295 }, 0.2, 40)){
        return true;
    }
    return false;
}


const DonutBerriesMatcher& DONUT_BERRIES_MATCHER(){
    static DonutBerriesMatcher matcher;
    return matcher;
}
DonutBerriesMatcher::DonutBerriesMatcher(const std::vector<double>& min_euclidean_distance)
    : CroppedImageDictionaryMatcher({0, 1})
{
    for (double x : min_euclidean_distance){
        m_min_euclidean_distance_squared.emplace_back(x * x);
    }
    for (const auto& item : DONUT_BERRIES_DATABASE()){
        add(item.first, item.second.sprite);
    }
}
auto DonutBerriesMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
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


const DonutBerriesOCR& DonutBerriesOCR::instance(){
    static DonutBerriesOCR reader;
    return reader;
}
DonutBerriesOCR::DonutBerriesOCR()
    : SmallDictionaryMatcher("PokemonLZA/Donuts/donut_berry_ocr.json")
{}
OCR::StringMatchResult DonutBerriesOCR::read_substring(
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


DonutBerriesReader::DonutBerriesReader(Color color)
    : m_color(color)
    , m_box_berry_text(berry_list_boxes(ImageFloatBox(0.065, 0.250, 0.173, 0.037)))
    , m_box_berry_icon(berry_list_boxes(ImageFloatBox(0.029, 0.254, 0.030, 0.047)))
{}

void DonutBerriesReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < BERRY_PAGE_LINES; c++){
        items.add(m_color, m_box_berry_text[c]);
        items.add(m_color, m_box_berry_icon[c]);
    }
}

std::array<ImageFloatBox, 8> DonutBerriesReader::berry_list_boxes(ImageFloatBox initial_box){
    std::array<ImageFloatBox, 8> material_boxes;
    double x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double initial_y = initial_box.y;
    double y_spacing = 0.086;
    for (size_t i = 0; i < 8; i++){
        double y = initial_y + i*y_spacing;
        material_boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return material_boxes;
}


ImageMatch::ImageMatchResult DonutBerriesReader::read_berry_page_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const{
    return read_with_icon_matcher(screen, m_box_berry_icon[index]);
}

ImageMatch::ImageMatchResult DonutBerriesReader::read_with_icon_matcher(const ImageViewRGB32& screen, const ImageFloatBox icon_box) const{
    // Get a crop of the berry icon
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
//    image.save("image" + std::to_string(icon_box.x) + ".png");

    ImageMatch::ImageMatchResult results;
    results = DONUT_BERRIES_MATCHER().match(image, ALPHA_SPREAD);
//  results.clear_beyond_alpha(MAX_ALPHA);

    return results;
}

OCR::StringMatchResult DonutBerriesReader::read_berry_page_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    size_t index
) const{
    return read_with_ocr(screen, logger, language, m_box_berry_text[index]);
}

OCR::StringMatchResult DonutBerriesReader::read_with_ocr(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    const ImageFloatBox icon_box
) const{

    // Get a crop of the berry text
    ImageViewRGB32 image = extract_box_reference(screen, icon_box);
    //image.save("image.png");

    OCR::StringMatchResult results;
    results = DonutBerriesOCR::instance().read_substring(logger, language, image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());

    return results;
}

}
}
}
