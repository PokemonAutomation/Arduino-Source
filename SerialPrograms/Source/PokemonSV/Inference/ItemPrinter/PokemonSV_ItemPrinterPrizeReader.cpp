/*  Item Printer - Prize Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_ItemPrinterPrizeReader.h"

// #include <iostream>
// using std::cout;
// using std::endl;

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
        m_boxes_normal_quantity[c] = ImageFloatBox(0.649, 0.175 + c * 0.0764444, 0.034000, 0.060);
        m_boxes_bonus_quantity[c] = ImageFloatBox(0.649, 0.204 + c * 0.0764444, 0.034000, 0.060);
    }
}

void ItemPrinterPrizeReader::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < 10; c++){
        items.add(COLOR_GREEN, m_boxes_normal[c]);
        items.add(COLOR_BLUE, m_boxes_bonus[c]);
        items.add(COLOR_GREEN, m_boxes_normal_quantity[c]);
        items.add(COLOR_BLUE, m_boxes_bonus_quantity[c]);
    }
}
std::array<std::string, 10> ItemPrinterPrizeReader::read_prizes(
    Logger& logger, const ImageViewRGB32& screen
) const{
    //  OCR 20 things in parallel.
    OCR::StringMatchResult results[20];

    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            if (index < 10){
                results[index] = ItemPrinterPrizeOCR::instance().read_substring(
                    nullptr, m_language,
                    extract_box_reference(screen, m_boxes_normal[index]),
                    OCR::WHITE_TEXT_FILTERS()
                );
            }else{
                results[index] = ItemPrinterPrizeOCR::instance().read_substring(
                    nullptr, m_language,
                    extract_box_reference(screen, m_boxes_bonus[index - 10]),
                    OCR::WHITE_TEXT_FILTERS()
                );
            }
        },
        0, 20, 1
    );

    std::array<std::string, 10> ret;
    for (size_t c = 0; c < 10; c++){
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


std::array<int16_t, 10> ItemPrinterPrizeReader::read_quantity(
    Logger& logger, const ImageViewRGB32& screen
) const{
    size_t total_rows = 10;

    // determine whether to use normal or bonus boxes for OCR
    double total_average_sum_normal = 0;
    double total_average_sum_bonus = 0;
    for (size_t i = 0; i < total_rows; i++){
        total_average_sum_normal += average_sum_filtered(screen, m_boxes_normal[i]);
        total_average_sum_bonus += average_sum_filtered(screen, m_boxes_bonus[i]);
    }

    logger.log("total_average_sum_normal: " + std::to_string(total_average_sum_normal));
    logger.log("total_average_sum_bonus: " + std::to_string(total_average_sum_bonus));

    if (total_average_sum_bonus > total_average_sum_normal){
        logger.log("Read quantity with bonus mode.");
    }else{
        logger.log("Read quantity with normal mode.");
    }

    const std::array<ImageFloatBox, 10>& boxes = (total_average_sum_bonus > total_average_sum_normal) 
                                                    ? m_boxes_bonus_quantity 
                                                    : m_boxes_normal_quantity;

    std::array<int16_t, 10> results;

    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            results[index] = read_number(logger, screen, boxes[index], (int8_t)index);
        },
        0, total_rows, 1
    );

    return results;
}


int16_t ItemPrinterPrizeReader::read_number(
    Logger& logger, 
    const ImageViewRGB32& screen, 
    const ImageFloatBox& box,
    int8_t line_index
) const{

    ImageViewRGB32 cropped = extract_box_reference(screen, box);
    int16_t number = (int16_t)OCR::read_number_waterfill(logger, cropped, 0xff808000, 0xffffffff, true, line_index);

    if (number < 1 || number > 40){
        number = 1; // default to 1 if we can't read the prize quantity
    }
    return (int16_t)number;
}

double ItemPrinterPrizeReader::average_sum_filtered(const ImageViewRGB32& screen, const ImageFloatBox& box) const{
    ImageViewRGB32 cropped = extract_box_reference(screen, box);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        cropped,
        false,
        0xff808000, 0xffffffff
    );    

    return image_stats(filtered).average.sum();
}


}
}
}
