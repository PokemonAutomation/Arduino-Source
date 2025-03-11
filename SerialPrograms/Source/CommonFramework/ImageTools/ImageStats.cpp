/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqr.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "ImageBoxes.h"
#include "ImageStats.h"

#include <iostream>
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

    FloatPixel variance = (sqr - sum*sum / (double)sums.count) / ((double)sums.count - 1);
    FloatPixel stddev = FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );

    ImageStats stats(average, stddev, sums.count);

    if (PreloadSettings::debug().COLOR_CHECK){
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

    for (size_t c = 0; c < w; c++){
        FloatPixel p(image.pixel(c, 0));
        sum += p;
        sqr_sum += p * p;
    }
    for (size_t c = 0; c < w; c++){
        FloatPixel p(image.pixel(c, h - 1));
        sum += p;
        sqr_sum += p * p;
    }
    for (size_t r = 0; r < h; r++){
        FloatPixel p(image.pixel(0, r));
        sum += p;
        sqr_sum += p * p;
    }
    for (size_t r = 0; r < h; r++){
        FloatPixel p(image.pixel(0, h - 1));
        sum += p;
        sqr_sum += p * p;
    }

    size_t total = 2 * (w + h);
    double totalf = (double)total;
    FloatPixel variance = (sqr_sum - sum*sum / totalf) / (totalf - 1);
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
