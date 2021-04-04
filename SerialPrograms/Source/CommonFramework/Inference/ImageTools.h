/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageTools_H
#define PokemonAutomation_CommonFramework_ImageTools_H

#include <ostream>
#include <QImage>
#include "InferenceTypes.h"
#include "FloatPixel.h"
#include "FillGeometry.h"

namespace PokemonAutomation{



InferenceBox translate_to_parent(
    const QImage& original_image,
    const InferenceBox& inference_box,
    const PixelBox& box
);

QImage extract_box(const QImage& image, const PixelBox& box);
QImage extract_box(const QImage& image, const InferenceBox& box);

double image_diff(const QImage& x, const QImage& y);

FloatPixel pixel_average(const QImage& image);
FloatPixel pixel_average_normalized(const QImage& image);
FloatPixel pixel_stddev(const QImage& image);


struct ImageStats{
    FloatPixel average;
    FloatPixel stddev;
};
ImageStats pixel_stats(const QImage& image);
ImageStats object_stats(const QImage& image, const FillMatrix& matrix, const FillGeometry& object);


bool is_black(const QImage& image, double max_rgb_sum = 100, double max_stddev_sum = 10);



}
#endif

