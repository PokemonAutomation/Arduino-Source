/*  Menu Option Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_MenuOptionReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



MenuOptionReader& MenuOptionReader::instance(){
    static MenuOptionReader reader;
    return reader;
}

MenuOptionReader::MenuOptionReader()
    : SmallDictionaryMatcher("PokemonSV/MenuOptionsOCR.json")
{}

OCR::StringMatchResult MenuOptionReader::read_substring(
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



}
}
}
