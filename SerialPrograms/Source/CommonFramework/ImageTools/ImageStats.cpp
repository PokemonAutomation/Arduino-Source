/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "ImageBoxes.h"
#include "ImageStats.h"

namespace PokemonAutomation{


ImageStats image_stats(const QImage& image){
    pxint_t w = image.width();
    pxint_t h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (pxint_t r = 0; r < h; r++){
        for (pxint_t c = 0; c < w; c++){
            FloatPixel p(image.pixel(c, r));
            sum += p;
            sqr_sum += p * p;
        }
    }
    size_t total = (size_t)w * (size_t)h;
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
