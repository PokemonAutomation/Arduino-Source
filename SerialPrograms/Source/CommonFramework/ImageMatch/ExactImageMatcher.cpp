/*  Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "ImageDiff.h"
#include "ExactImageMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


ExactImageMatcher::ExactImageMatcher(QImage image)
    : m_image(std::move(image))
    , m_stats(image_stats(m_image))
{
    if (m_image.isNull()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is null.");
    }
//    cout << m_stats.stddev.sum() << endl;
}

void ExactImageMatcher::process_images(QImage& reference, const ImageViewRGB32& image) const{
//    cout << "ExactImageMatcher::process_images(): " << image.width() << " x " << image.height() << endl;
    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_stats.average;

//    cout << image_brightness << m_stats.average << scale << endl;

    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.8, 1.2);

    reference = m_image;
    scale_brightness(reference, scale);
}


double ExactImageMatcher::rmsd(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }

//    cout << "ExactImageMatcher::rmsd(): image = " << image.width() << " x " << image.height() << endl;
    QImage scaled = image.scaled_to_QImage(m_image.width(), m_image.height());
//    cout << "ExactImageMatcher::rmsd(): scaled = " << scaled.width() << " x " << scaled.height() << endl;
    QImage reference;
    process_images(reference, scaled);

#if 0
    static int c = 0;
    image.save("test-" + QString::number(c) + "-image.png");
    reference.save("test-" + QString::number(c) + "-sprite.png");
    c++;
#endif

    double rmsd = pixel_RMSD(reference, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd;
}
double ExactImageMatcher::rmsd(const ImageViewRGB32& image, QRgb background) const{
    if (!image){
        return 1000.;
    }
    QImage scaled = image.scaled_to_QImage(m_image.width(), m_image.height());
    QImage reference;
    process_images(reference, scaled);
    return pixel_RMSD(reference, scaled, background);
}
double ExactImageMatcher::rmsd_masked(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }
    QImage scaled = image.scaled_to_QImage(m_image.width(), m_image.height());
    QImage reference;
    process_images(reference, scaled);
    return pixel_RMSD_masked(reference, scaled);
}




WeightedExactImageMatcher::WeightedExactImageMatcher(QImage image, const InverseStddevWeight& weight)
    : ExactImageMatcher(std::move(image))
    , m_multiplier(1. / (m_stats.stddev.sum() * weight.stddev_coefficient + weight.offset))
{}


double WeightedExactImageMatcher::diff(const ImageViewRGB32& image) const{
    if (!image){
        return 1000.;
    }
    return rmsd(image) * m_multiplier;
}
double WeightedExactImageMatcher::diff(const ImageViewRGB32& image, QRgb background) const{
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
