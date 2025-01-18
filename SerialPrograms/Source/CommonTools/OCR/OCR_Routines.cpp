/*  OCR Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "OCR_RawOCR.h"
#include "OCR_DictionaryMatcher.h"
#include "OCR_Routines.h"

namespace PokemonAutomation{
namespace OCR{


StringMatchResult multifiltered_OCR(
    Language language, const DictionaryMatcher& dictionary, const ImageViewRGB32& image,
    const std::vector<TextColorRange>& text_color_ranges,
    double log10p_spread,
    double min_text_ratio, double max_text_ratio
){
    if (image.width() == 0 || image.height() == 0){
        return StringMatchResult();
    }

    std::vector<BlackWhiteRgb32Range> bw_filters;
    for (const auto& range : text_color_ranges){
        bw_filters.emplace_back(BlackWhiteRgb32Range{range.mins, range.maxs, true});
    }
    std::vector<std::pair<ImageRGB32, size_t>> filtered_images = to_blackwhite_rgb32_range(image, bw_filters);

    double pixels_inv = 1. / (image.width() * image.height());

    //  Run all the filters.
    StringMatchResult ret;
//    int c = 0;
    for (const auto& filtered : filtered_images){
        std::string text = ocr_read(language, filtered.first);
//        cout << text.toStdString() << endl;
//        filtered.first.save("test" + QString::number(c++) + ".png");

        //  Compute ratio of image that matches text color. Skip if it's out of range.
        double ratio = filtered.second * pixels_inv;
//        cout << "ratio = " << ratio << endl;
        if (ratio < min_text_ratio || ratio > max_text_ratio){
            continue;
        }

        StringMatchResult current = dictionary.match_substring(language, text, log10p_spread);
        ret.exact_match |= current.exact_match;
        ret.results.insert(current.results.begin(), current.results.end());
    }

//    ret.log(global_logger_tagged(), -1.5);

    //  Prune duplicates.
    std::set<std::string> tokens;
    for (auto iter = ret.results.begin(); iter != ret.results.end();){
        if (tokens.find(iter->second.token) == tokens.end()){
            tokens.insert(iter->second.token);
            ++iter;
        }else{
            iter = ret.results.erase(iter);
        }
    }

    ret.clear_beyond_spread(log10p_spread);
    return ret;
}



const std::vector<TextColorRange>& BLACK_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xff000000, 0xff404040},
        {0xff000000, 0xff606060},
        {0xff000000, 0xff808080},
    };
    return filters;
}
const std::vector<TextColorRange>& WHITE_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xff808080, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
    };
    return filters;
}
const std::vector<TextColorRange>& BLACK_OR_WHITE_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xff000000, 0xff404040},
        {0xff000000, 0xff606060},
        {0xff000000, 0xff808080},
        {0xff808080, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
    };
    return filters;
}
const std::vector<TextColorRange>& BLUE_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xFF3287A2, 0xFFA7F4FF},
        {0xFF2283A2, 0xFF8EF1FF},
        {0xFF428BA2, 0xFFC1F7FF},
    };
    return filters;
}








}
}
