/*  Pokemon Moves Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSV_PokemonMovesReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



PokemonMovesOCR& PokemonMovesOCR::instance(){
    static PokemonMovesOCR reader;
    return reader;
}

PokemonMovesOCR::PokemonMovesOCR()
    : SmallDictionaryMatcher("PokemonSV/PokemonMovesOCR.json")
{}

OCR::StringMatchResult PokemonMovesOCR::read_substring(
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

PokemonMovesReader::PokemonMovesReader(Language language)
    : m_language(language)
{
    for (size_t c = 0; c < 4; c++){
        m_boxes_rearrange[c] = ImageFloatBox(0.345, 0.245 + c * 0.074, 0.250, 0.065); 
    }
}

void PokemonMovesReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < 4; c++){
        items.add(COLOR_GREEN, m_boxes_rearrange[c]);
    }
}

std::string PokemonMovesReader::read_move(Logger& logger, const ImageViewRGB32& screen, int8_t index) const{
    ImageViewRGB32 cropped = extract_box_reference(screen, m_boxes_rearrange[index]);
    const auto ocr_result = PokemonMovesOCR::instance().read_substring(
        logger, m_language, 
        cropped, OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );

    std::multimap<double, OCR::StringMatchData> results;
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
    }

    if (results.empty()){
        return "";
    }

    if (results.size() > 1){
        throw_and_log<OperationFailedException>(
            logger, ErrorReport::SEND_ERROR_REPORT,
            "MenuOption::read_option(): Unable to read item. Ambiguous or multiple results.\n" + language_warning(m_language)
        );
    }

    return results.begin()->second.token;
}



}
}
}
