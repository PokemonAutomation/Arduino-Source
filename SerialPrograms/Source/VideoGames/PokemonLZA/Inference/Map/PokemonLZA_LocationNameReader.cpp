/*  Location Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonLZA_LocationNameReader.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


LocationNameOCR& LocationNameOCR::instance(){
    static LocationNameOCR reader;
    return reader;
}


LocationNameOCR::LocationNameOCR()
    : SmallDictionaryMatcher("PokemonLZA/LocationName.json")
{}

OCR::StringMatchResult LocationNameOCR::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
    ) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
        );
}

LocationNameReader::LocationNameReader(Color color)
    : m_color(color)
    , m_box_location_name(get_all_box_locations(ImageFloatBox(0.035, 0.254, 0.287, 0.050)))
{}

void LocationNameReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < PAGE_SIZE; c++){
        items.add(m_color, m_box_location_name[c]);
    }
}

std::array<ImageFloatBox, LocationNameReader::PAGE_SIZE> LocationNameReader::get_all_box_locations(ImageFloatBox initial_box){
    std::array<ImageFloatBox, LocationNameReader::PAGE_SIZE> material_boxes;
    double x = initial_box.x;
    double width = initial_box.width;
    double height = initial_box.height;
    double initial_y = initial_box.y;
    double y_spacing = 0.078;
    for (size_t i = 0; i < LocationNameReader::PAGE_SIZE; i++){
        double y = initial_y + i*y_spacing;
        material_boxes[i] = ImageFloatBox(x, y, width, height);
    }
    return material_boxes;
}

OCR::StringMatchResult LocationNameReader::read_location_name(
    const ImageViewRGB32& screen, 
    Logger& logger, 
    Language language, 
    size_t index
) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_box_location_name[index]);

    OCR::StringMatchResult results;
    results = LocationNameOCR::instance().read_substring(logger, language, image, OCR::BLACK_OR_WHITE_TEXT_FILTERS());

    return results;
}

}
}
}
