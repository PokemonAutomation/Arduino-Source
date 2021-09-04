/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageTools_H
#define PokemonAutomation_CommonFramework_ImageTools_H

#include <ostream>
#include <QImage>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{



ImageFloatBox translate_to_parent(
    const QImage& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
);



double image_diff_total(const QImage& x, const QImage& y);
QImage image_diff_greyscale(const QImage& x, const QImage& y);

FloatPixel pixel_average(const QImage& image);
FloatPixel pixel_average_normalized(const QImage& image);
FloatPixel pixel_stddev(const QImage& image);


ImageStats object_stats(const QImage& image, const CellMatrix& matrix, const FillGeometry& object);




}
#endif

