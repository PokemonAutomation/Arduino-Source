/*  Cropped Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "ImageCropper.h"
//#include "ImageDiff.h"
#include "CroppedImageDictionaryMatcher.h"

#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{



CroppedImageDictionaryMatcher::CroppedImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight)
    : m_weight(weight)
{}
void CroppedImageDictionaryMatcher::add(const std::string& slug, const ImageViewRGB32& image){
    if (!image){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Null image.");
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate slug: " + slug);
    }

    ImageViewRGB32 cropped = trim_image_alpha(image);

#if 0
    if (slug == "hamburger"){
        image.save("matcher-original.png");
        cropped.save("matcher-cropped.png");
    }
#endif

    iter = m_database.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(slug),
        std::forward_as_tuple(cropped.copy(), m_weight)
    ).first;
//    cout << iter->first << ": " << iter->second.stats().stddev.sum() << endl;
}



ImageMatchResult CroppedImageDictionaryMatcher::match(
    const ImageViewRGB32& image,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (!image){
        return results;
    }

    if (PreloadSettings::debug().IMAGE_DICTIONARY_MATCHING){
        std::cout << "CroppedImageDictionaryMatcher: match input image: " << std::endl;
        dump_debug_image(global_logger_command_line(), "CommonFramework/CroppedImageDictionaryMatcher", "match_input", image);
    }

    std::vector<ImageViewRGB32> crops = get_crop_candidates(image);

    if (PreloadSettings::debug().IMAGE_DICTIONARY_MATCHING){
//        size_t c = 0;
        for (const ImageViewRGB32& crop : crops){
//            std::cout << "CroppedImageDictionaryMatcher: process input" << c << "image with background " << crop.background.to_string() << std::endl;
            dump_debug_image(global_logger_command_line(), "CommonFramework/CroppedImageDictionaryMatcher", "match_input_processed", crop);
//            c++;
        }
    }



    for (const auto& item : m_database){
        for (const ImageViewRGB32& crop : crops){
            double alpha = item.second.diff(crop);
            results.add(alpha, item.first);
            results.clear_beyond_spread(alpha_spread);
        }
    }



#if 0
    Color background;
    ImageViewRGB32 processed = cropped[0].image;
    background = cropped[0].background;

//    ImageRGB32 processed = process_image(image, background);
    if (PreloadSettings::debug().IMAGE_DICTIONARY_MATCHING){
        std::cout << "CroppedImageDictionaryMatcher: process input image with background " << background.to_string() << std::endl;
        dump_debug_image(global_logger_command_line(), "CommonFramework/CroppedImageDictionaryMatcher", "match_input_processed", processed);
    }

    processed.save("processed.png");

    for (const auto& item : m_database){
#if 0
        if (item.first != "onion"){
            continue;
        }
#endif
        double alpha = item.second.diff(processed);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }
#endif

    if (PreloadSettings::debug().IMAGE_DICTIONARY_MATCHING){
        std::cout << "CroppedImageDictionaryMatcher: results: " << std::endl;
        size_t count = 0;
        for(const auto& result : results.results){
            std::cout << "alpha=" << result.first << ", " << result.second << std::endl;
            const auto& image_template = m_database.find(result.second)->second.image_template();
            dump_debug_image(global_logger_command_line(), "CommonFramework/CroppedImageDictionaryMatcher", "match_result_" + std::to_string(count) + "_" + result.second, image_template);
            ++count;
        }
    }
    return results;
}






}
}
