/*  Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "ImageDiff.h"
#include "ExactImageMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


ExactImageMatcher::ExactImageMatcher(QImage image, void*)
    : m_image(std::move(image))
    , m_stats(image_stats(m_image))
{
//    cout << m_stats.stddev.sum() << endl;
}

void ExactImageMatcher::process_images(QImage& reference, QImage& image) const{
    if (image.size() != m_image.size()){
        image = image.scaled(m_image.size());
    }

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


double ExactImageMatcher::rmsd(QImage image) const{
    QImage reference;
    process_images(reference, image);
//    image.save("image.png");
//    reference.save("sprite.png");
    return pixel_RMSD(reference, image);
}
double ExactImageMatcher::rmsd(QImage image, QRgb background) const{
    QImage reference;
    process_images(reference, image);
    return pixel_RMSD(reference, image, background);
}
double ExactImageMatcher::rmsd_masked(QImage image) const{
    QImage reference;
    process_images(reference, image);
    return pixel_RMSD_masked(reference, image);
}




WeightedExactImageMatcher::WeightedExactImageMatcher(QImage image, const InverseStddevWeight& weight)
    : ExactImageMatcher(std::move(image), nullptr)
    , m_multiplier(1. / (m_stats.stddev.sum() * weight.stddev_coefficient + weight.offset))
{}


double WeightedExactImageMatcher::diff(QImage image) const{
    return rmsd(std::move(image)) * m_multiplier;
}
double WeightedExactImageMatcher::diff(QImage image, QRgb background) const{
    return rmsd(std::move(image), background) * m_multiplier;
}
double WeightedExactImageMatcher::diff_masked(QImage image) const{
    return rmsd_masked(std::move(image)) * m_multiplier;
}


















}
}
