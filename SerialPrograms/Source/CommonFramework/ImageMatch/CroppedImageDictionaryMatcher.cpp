/*  Cropped Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ImageCropper.h"
//#include "ImageDiff.h"
#include "CroppedImageDictionaryMatcher.h"

//#include <iostream>
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

    Color background;
    ImageRGB32 processed = process_image(image, background);
//    cout << (uint32_t)background << endl;

    for (const auto& item : m_database){
#if 0
        if (item.first != "onion"){
            continue;
        }
#endif
        double alpha = item.second.diff(processed, background);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }

    return results;
}






}
}
