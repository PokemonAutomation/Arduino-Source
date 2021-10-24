/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqr.h"
#include "ImageBoxes.h"
#include "ImageStats.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



FloatPixel image_average(const QImage& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine()
    );

    ImageStats stats;
    FloatPixel sum(sums.sumR, sums.sumG, sums.sumB);

    return sum / sums.count;
}
FloatPixel image_stddev(const QImage& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine()
    );

    FloatPixel sum(sums.sumR, sums.sumG, sums.sumB);
    FloatPixel sqr(sums.sqrR, sums.sqrG, sums.sqrB);

    FloatPixel variance = (sqr - sum*sum / sums.count) / (sums.count - 1);

    return FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );
}
ImageStats image_stats(const QImage& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine()
    );

    ImageStats stats;
    FloatPixel sum(sums.sumR, sums.sumG, sums.sumB);
    FloatPixel sqr(sums.sqrR, sums.sqrG, sums.sqrB);

    stats.average = sum / sums.count;

    FloatPixel variance = (sqr - sum*sum / sums.count) / (sums.count - 1);
    stats.stddev = FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );

    return stats;
}





ImageStats image_border_stats(const QImage& image){
    pxint_t w = image.width();
    pxint_t h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }

    FloatPixel sum;
    FloatPixel sqr_sum;

    for (pxint_t c = 0; c < w; c++){
        FloatPixel p(image.pixel(c, 0));
        sum += p;
        sqr_sum += p * p;
    }
    for (pxint_t c = 0; c < w; c++){
        FloatPixel p(image.pixel(c, h - 1));
        sum += p;
        sqr_sum += p * p;
    }
    for (pxint_t r = 0; r < h; r++){
        FloatPixel p(image.pixel(0, r));
        sum += p;
        sqr_sum += p * p;
    }
    for (pxint_t r = 0; r < h; r++){
        FloatPixel p(image.pixel(0, h - 1));
        sum += p;
        sqr_sum += p * p;
    }

    size_t total = 2 * (w + h);
    FloatPixel variance = (sqr_sum - sum*sum / total) / (total - 1);
    return ImageStats{
        sum / total,
        FloatPixel(
            std::sqrt(variance.r),
            std::sqrt(variance.g),
            std::sqrt(variance.b)
        )
    };
}



}
