/*  Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "ImageDiff.h"
#include "ImageMatchMetadata.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


ExactMatchMetadata::ExactMatchMetadata(QImage image)
    : m_image(std::move(image))
    , m_average_pixel(pixel_average(m_image, m_image))
    , m_max_possible_RMSD(pixel_max_possible_RMSD(m_image))
{}

double ExactMatchMetadata::rmsd_ratio_with(QImage image) const{
    if (image.size() != m_image.size()){
        image = image.scaled(m_image.size());
    }

    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_average_pixel;

    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.8, 1.2);

    QImage processed_target = m_image;
    scale_brightness(processed_target, scale);

    return pixel_RMSD(processed_target, image) / m_max_possible_RMSD;
}
double ExactMatchMetadata::rmsd_ratio_with(QImage image, QRgb background) const{
    if (image.size() != m_image.size()){
        image = image.scaled(m_image.size());
    }

    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_average_pixel;

    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.8, 1.2);

    QImage processed_target = m_image;
    scale_brightness(processed_target, scale);

    return pixel_RMSD(processed_target, image, background) / m_max_possible_RMSD;
}
double ExactMatchMetadata::rmsd_ratio_masked_with(QImage image) const{
    if (image.size() != m_image.size()){
        image = image.scaled(m_image.size());
    }

    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_average_pixel;

    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.8, 1.2);

    QImage processed_target = m_image;
    scale_brightness(processed_target, scale);

    image.save("processed_image.png");
    processed_target.save("processed_target.png");

    return pixel_RMSD_masked(processed_target, image) / m_max_possible_RMSD;
}



}
}
