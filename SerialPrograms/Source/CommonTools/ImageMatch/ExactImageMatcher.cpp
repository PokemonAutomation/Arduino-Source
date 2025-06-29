/*  Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "ExactImageMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


ExactImageMatcher::ExactImageMatcher(ImageRGB32 image)
    : m_image(std::move(image))
    , m_stats(image_stats(m_image))
{
    if (!m_image){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is null.");
    }
//    cout << m_stats.stddev.sum() << endl;
}

ImageRGB32 ExactImageMatcher::scale_template_brightness(const ImageViewRGB32& image) const{
    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_stats.average;

    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.85, 1.15);

    ImageRGB32 ret = m_image.copy();
    scale_brightness(ret, scale);
//    ret.save("test.png");
    return ret;
}


double ExactImageMatcher::rmsd(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }

//    image.save("test.png");

//    cout << "ExactImageMatcher::rmsd(): image = " << image.width() << " x " << image.height() << endl;
    ImageRGB32 scaled = image.scale_to(m_image.width(), m_image.height());
//    cout << "ExactImageMatcher::rmsd(): scaled = " << scaled.width() << " x " << scaled.height() << endl;
    ImageRGB32 reference = scale_template_brightness(scaled);

#if 0
    static int c = 0;
    image.save("test-" + std::to_string(c) + "-image.png");
    reference.save("test-" + std::to_string(c) + "-sprite.png");
    c++;
#endif

    double rmsd = pixel_RMSD(reference, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd;
}
double ExactImageMatcher::rmsd(const ImageViewRGB32& image, Color background) const{
    if (!image){
        return 1000.;
    }
    ImageRGB32 scaled = image.scale_to(m_image.width(), m_image.height());
    ImageRGB32 reference = scale_template_brightness(scaled);

#if 0
    static int c = 0;
    scaled.save("test-" + std::to_string(c) + "-image.png");
    reference.save("test-" + std::to_string(c) + "-sprite.png");
    c++;
#endif

    return pixel_RMSD(reference, scaled, background);
}
double ExactImageMatcher::rmsd_masked(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }
    ImageRGB32 scaled = image.scale_to(m_image.width(), m_image.height());
    ImageRGB32 reference = scale_template_brightness(scaled);
    return pixel_RMSD_masked(reference, scaled);
}




WeightedExactImageMatcher::WeightedExactImageMatcher(ImageRGB32 image, const InverseStddevWeight& weight)
    : ExactImageMatcher(std::move(image))
    , m_multiplier(1. / (m_stats.stddev.sum() * weight.stddev_coefficient + weight.offset))
{}


double WeightedExactImageMatcher::diff(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }
    return rmsd(image) * m_multiplier;
}
double WeightedExactImageMatcher::diff(const ImageViewRGB32& image, Color background) const{
    if (!image){
        return 1000.;
    }
    return rmsd(image, background) * m_multiplier;
}
double WeightedExactImageMatcher::diff_masked(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }
    return rmsd_masked(image) * m_multiplier;
}


















}
}
