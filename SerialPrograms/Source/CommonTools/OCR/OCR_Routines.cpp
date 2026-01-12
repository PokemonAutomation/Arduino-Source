/*  OCR Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/ImageFilter.h"
#include "ML/Inference/ML_PaddleOCRPipeline.h"
#include "OCR_RawOCR.h"
#include "OCR_DictionaryMatcher.h"
#include "OCR_Routines.h"

// #include <iostream>
// using std::cout;
// using std::endl;

static constexpr bool USE_PADDLE_OCR = true;


namespace PokemonAutomation{
namespace OCR{


StringMatchResult multifiltered_OCR(
    Language language, const DictionaryMatcher& dictionary, const ImageViewRGB32& image,
    const std::vector<TextColorRange>& text_color_ranges,
    double log10p_spread,
    double min_text_ratio, double max_text_ratio,
    OCR::PageSegMode psm
){
    if (image.width() == 0 || image.height() == 0){
        return StringMatchResult();
    }

    std::vector<BlackWhiteRgb32Range> bw_filters;
    for (const auto& range : text_color_ranges){
        bw_filters.emplace_back(
            BlackWhiteRgb32Range{true, range.mins, range.maxs}
        );
    }
    std::vector<std::pair<ImageRGB32, size_t>> filtered_images = to_blackwhite_rgb32_range(image, bw_filters);

    double pixels_inv = 1. / (image.width() * image.height());

    #if 1
    ML::PaddleOCRPipeline paddle_ocr(language);
    #endif

    //  Run all the filters.
    SpinLock lock;
    StringMatchResult ret;
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            const std::pair<ImageRGB32, size_t>& filtered = filtered_images[index];

            std::string text;
            if (USE_PADDLE_OCR){
                text = paddle_ocr.Recognize(filtered.first);
            }else{
                text = ocr_read(language, filtered.first, psm);
            }
            
            // cout << "multifiltered_OCR: " << index << " -> " << text << endl;
            // filtered.first.save("test_" + std::to_string(index) + ".png");

            //  Compute ratio of image that matches text color. Skip if it's out of range.
            double ratio = filtered.second * pixels_inv;
//            cout << "ratio = " << ratio << endl;
            if (ratio < min_text_ratio || ratio > max_text_ratio){
                return;
            }

            StringMatchResult current = dictionary.match_substring(language, text, log10p_spread);

            WriteSpinLock lg(lock);
            ret.exact_match |= current.exact_match;
            ret.results.insert(current.results.begin(), current.results.end());

        },
        0, filtered_images.size(), 1
    );
//    int c = 0;
//    for (const auto& filtered : filtered_images){
//    }

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


StringMatchResult dictionary_OCR(
    Language language, const DictionaryMatcher& dictionary, const ImageViewRGB32& image,
    double log10p_spread, OCR::PageSegMode psm
){
    if (image.width() == 0 || image.height() == 0){
        return StringMatchResult();
    }

    //  Run all the filters.
    std::string text;
    if (USE_PADDLE_OCR){
        ML::PaddleOCRPipeline paddle_ocr(language);
        text = paddle_ocr.Recognize(image);
    }else{
        text = ocr_read(language, image, psm);
    }

    // cout << "dictionary_OCR: " << text << endl;
    // image.save("test_dictionary_OCR.png");

    return dictionary.match_substring(language, text, log10p_spread);
}



const std::vector<TextColorRange>& BLACK_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xff000000, 0xff404040},
        {0xff000000, 0xff606060},
        {0xff000000, 0xff808080},
        {0xff000000, 0xff909090},
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
const std::vector<TextColorRange>& LIME_TEXT_FILTERS(){
    static std::vector<TextColorRange> filters{
        {0xff6FA60D, 0xffBEEF36},
        {0xff5E9400, 0xffC9F95C},
        {0xff4D8100, 0xffD3FB7E},
    };
    return filters;
}








}
}
