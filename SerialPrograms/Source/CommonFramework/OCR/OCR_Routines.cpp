/*  OCR Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "OCR_RawOCR.h"
#include "OCR_DictionaryMatcher.h"
#include "OCR_Routines.h"

namespace PokemonAutomation{
namespace OCR{


StringMatchResult multifiltered_OCR(
    Language language, const DictionaryMatcher& dictionary, const ConstImageRef& image,
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
    std::vector<std::pair<QImage, size_t>> filtered_images = to_blackwhite_rgb32_range(image, bw_filters);

    double pixels_inv = 1. / (image.width() * image.height());

    //  Run all the filters.
    StringMatchResult ret;
//    int c = 0;
    for (const auto& filtered : filtered_images){
        QString text = ocr_read(language, filtered.first);
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





}
}
