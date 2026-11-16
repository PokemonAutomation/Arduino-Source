/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <iostream>
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqr.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ImageBoxes.h"

#include "ImageStats.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{



FloatPixel image_average(const ImageViewRGB32& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        image.data(), image.bytes_per_row(),
        image.data(), image.bytes_per_row()
    );

    FloatPixel sum((double)sums.sumR, (double)sums.sumG, (double)sums.sumB);

    return sum / (double)sums.count;
}
FloatPixel image_stddev(const ImageViewRGB32& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        image.data(), image.bytes_per_row(),
        image.data(), image.bytes_per_row()
    );

    FloatPixel sum((double)sums.sumR, (double)sums.sumG, (double)sums.sumB);
    FloatPixel sqr((double)sums.sqrR, (double)sums.sqrG, (double)sums.sqrB);

    FloatPixel variance = (sqr - sum*sum / (double)sums.count) / ((double)sums.count - 1);

    return FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );
}
ImageStats image_stats(const ImageViewRGB32& image){
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        image.data(), image.bytes_per_row(),
        image.data(), image.bytes_per_row()
    );

    FloatPixel sum((double)sums.sumR, (double)sums.sumG, (double)sums.sumB);
    FloatPixel sqr((double)sums.sqrR, (double)sums.sqrG, (double)sums.sqrB);

    FloatPixel average = sum / (double)sums.count;

    // (\sum (x_i^2) - (\sum x_i)^2 / n) / (n-1)
    FloatPixel variance = (sqr - sum*sum / (double)sums.count) / ((double)sums.count - 1);
    FloatPixel stddev = FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );

    ImageStats stats(average, stddev, sums.count);

    if (STATIC_GLOBALS.COLOR_CHECK){
        std::cout << "Compute imageStats: avg " << stats.average.to_string() << " (sum " << stats.average.sum()
                  << ") stddev " << stats.stddev.to_string() << " (sum " << stats.stddev.sum()
                  << ") count " << stats.count << std::endl;
    }

    return stats;
}





ImageStats image_border_stats(const ImageViewRGB32& image){
    size_t w = image.width();
    size_t h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }

    FloatPixel sum;
    FloatPixel sqr_sum;
    size_t total = 0;
    auto add_pixel = [&](size_t x, size_t y){
        FloatPixel p(image.pixel(x, y));
        sum += p;
        sqr_sum += p * p;
        total++;
    };

    //  Top and bottom rows, then the left and right columns without the corners.
    //  The checks on h and w keep a one-pixel-tall or one-pixel-wide image from
    //  counting the same pixels twice.
    for (size_t c = 0; c < w; c++){
        add_pixel(c, 0);
        if (h > 1){
            add_pixel(c, h - 1);
        }
    }
    for (size_t r = 1; r + 1 < h; r++){
        add_pixel(0, r);
        if (w > 1){
            add_pixel(w - 1, r);
        }
    }

    const double totalf = (double)total;
    const FloatPixel variance = (sqr_sum - sum*sum / totalf) / (totalf - 1);
    return ImageStats{
        sum / totalf,
        FloatPixel(
            std::sqrt(variance.r),
            std::sqrt(variance.g),
            std::sqrt(variance.b)
        ),
        total
    };
}



}
