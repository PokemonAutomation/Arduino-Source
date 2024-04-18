/*  Item Printer - Prize Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/Logging/Logger.h"
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
        Logger* logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const{
        return match_substring_from_image_multifiltered(
            logger, language, image, text_color_ranges,
            MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
        );
    }

};




ItemPrinterPrizeReader::ItemPrinterPrizeReader(Language language)
    : m_language(language)
{
    for (size_t c = 0; c < 10; c++){
        m_boxes_normal[c] = ImageFloatBox(0.361, 0.175 + c * 0.0764444, 0.240, 0.060);
        m_boxes_bonus[c] = ImageFloatBox(0.361, 0.204 + c * 0.0764444, 0.240, 0.060);
    }
}

void ItemPrinterPrizeReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < 10; c++){
        items.add(COLOR_GREEN, m_boxes_normal[c]);
        items.add(COLOR_BLUE, m_boxes_bonus[c]);
    }
}
std::array<std::string, 10> ItemPrinterPrizeReader::read(
    Logger& logger, AsyncDispatcher& dispatcher,
    const ImageViewRGB32& screen
) const{
    OCR::StringMatchResult results[20];
    std::unique_ptr<AsyncTask> tasks[20];
    for (size_t c = 0; c < 10; c++){
        tasks[c] = dispatcher.dispatch([=, this, &results]{
            results[c] = ItemPrinterPrizeOCR::instance().read_substring(
                nullptr, m_language,
                extract_box_reference(screen, m_boxes_normal[c]),
                OCR::WHITE_TEXT_FILTERS()
            );
        });
    }
    for (size_t c = 0; c < 10; c++){
        tasks[10 + c] = dispatcher.dispatch([=, this, &results]{
            results[10 + c] = ItemPrinterPrizeOCR::instance().read_substring(
                nullptr, m_language,
                extract_box_reference(screen, m_boxes_bonus[c]),
                OCR::WHITE_TEXT_FILTERS()
            );
        });
    }

    for (size_t c = 0; c < 20; c++){
        tasks[c]->wait_and_rethrow_exceptions();
    }


    std::array<std::string, 10> ret;
    for (size_t c = 0; c < 10; c++){
//        OCR::StringMatchResult result = ItemPrinterPrizeOCR::instance().read_substring(
//            nullptr, m_language,
//            extract_box_reference(screen, m_boxes_normal[c]),
//            OCR::WHITE_TEXT_FILTERS()
//        );
        OCR::StringMatchResult& result = results[c];
        result.clear_beyond_log10p(MAX_LOG10P);
        result.clear_beyond_spread(MAX_LOG10P_SPREAD);
        result += results[10 + c];
        result.clear_beyond_log10p(MAX_LOG10P);
        result.clear_beyond_spread(MAX_LOG10P_SPREAD);
        result.log(logger, MAX_LOG10P);
        if (result.results.size() != 1){
            continue;
        }
        ret[c] = std::move(result.results.begin()->second.token);
    }
    return ret;
}





}
}
}
