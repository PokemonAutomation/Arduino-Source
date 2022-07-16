/*  Cropped Silhouette Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ImageCropper.h"
#include "ImageDiff.h"
#include "SilhouetteDictionaryMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


void SilhouetteDictionaryMatcher::add(const std::string& slug, QImage image){
    if (image.isNull()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Null image.");
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate slug: " + slug);
    }

    m_database.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(slug),
        std::forward_as_tuple(trim_image_alpha(image))
    );
}



ImageMatchResult SilhouetteDictionaryMatcher::match(
    const ImageViewRGB32& image,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (!image){
        return results;
    }

    QImage processed = image.to_QImage_owning();

    crop_image(processed);
    set_alpha_channels(processed);

    for (const auto& item : m_database){
//        if (item.first != "solosis"){
//            continue;
//        }
        double alpha = item.second.rmsd_masked(processed);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }

    return results;
}






}
}
