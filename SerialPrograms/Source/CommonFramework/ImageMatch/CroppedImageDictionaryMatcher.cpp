/*  Cropped Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exception.h"
#include "ImageCropper.h"
#include "ImageDiff.h"
#include "CroppedImageDictionaryMatcher.h"

#include "CommonFramework/Inference/ImageTools.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{



CroppedImageDictionaryMatcher::CroppedImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight)
    : m_weight(weight)
{}
void CroppedImageDictionaryMatcher::add(const std::string& slug, QImage image){
    if (image.isNull()){
        PA_THROW_StringException("Null image.");
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        PA_THROW_StringException("Duplicate slug: " + slug);
    }

    m_database.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(slug),
        std::forward_as_tuple(trim_image_alpha(image), m_weight)
    );
}



ImageMatchResult CroppedImageDictionaryMatcher::match(
    QImage image,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (image.isNull()){
        return results;
    }


    QRgb background = crop_image(image);
//    image.save("test.png");
//    cout << FloatPixel(background) << endl;
    set_alpha_channels(image);


    for (const auto& item : m_database){
//        if (item.first != "solosis"){
//            continue;
//        }
        double alpha = item.second.diff(image, background);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }

    return results;
}






}
}
