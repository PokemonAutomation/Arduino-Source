/*  Item Printer - Prize Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_ItemPrinterPrizeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




class ItemPrinterPrizeOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    ItemPrinterPrizeOCR()
        : SmallDictionaryMatcher("PokemonSV/ItemPrinterPrizeOCR.json")
    {}

    static ItemPrinterPrizeOCR& instance(){
        static ItemPrinterPrizeOCR reader;
        return reader;
    }

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const{
        return match_substring_from_image_multifiltered(
            &logger, language, image, text_color_ranges,
            MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
        );
    }

};




ItemPrinterPrizeReader::ItemPrinterPrizeReader(Language language)
    : m_language(language)
{
    for (size_t c = 0; c < 10; c++){
        m_boxes[c] = ImageFloatBox(0.361, 0.204 + c * 0.0764444, 0.240, 0.060);
    }
}

void ItemPrinterPrizeReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < 10; c++){
        items.add(COLOR_RED, m_boxes[c]);
    }
}
std::array<std::string, 10> ItemPrinterPrizeReader::read(Logger& logger, const ImageViewRGB32& screen) const{
    std::array<std::string, 10> ret;
    for (size_t c = 0; c < 10; c++){
#if 1   //  TODO, use the other version when we have the OCR file.
        ImageRGB32 filtered = to_blackwhite_rgb32_range(
            extract_box_reference(screen, m_boxes[c]),
            0xff808080, 0xffffffff,
            true
        );
        std::string text = OCR::ocr_read(m_language, filtered);
        logger.log("OCR: " + text);
        ret[c] = std::move(text);
#else
        OCR::StringMatchResult result = ItemPrinterPrizeOCR::instance().read_substring(
            logger, m_language,
            extract_box_reference(screen, m_boxes[c]),
            OCR::WHITE_TEXT_FILTERS()
        );
        result.clear_beyond_log10p(MAX_LOG10P);
        result.clear_beyond_log10p(MAX_LOG10P_SPREAD);
        if (result.results.size() != 1){
            continue;
        }
        ret[c] = std::move(result.results.begin()->second.token);
#endif
    }
    return ret;
}





}
}
}
