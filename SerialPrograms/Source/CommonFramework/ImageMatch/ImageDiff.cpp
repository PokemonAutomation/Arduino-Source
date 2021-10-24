/*  Image Diff
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <smmintrin.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/SIMDDebuggers.h"
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqr.h"
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqrDev.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "ImageDiff.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


FloatPixel pixel_average(const QImage& image, const QImage& alpha_mask){
    if (!image.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }
    if (image.size() != alpha_mask.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    Kernels::PixelSums sums;
    Kernels::pixel_sum_sqr(
        sums, image.width(), image.height(),
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        (const uint32_t*)alpha_mask.bits(), alpha_mask.bytesPerLine()
    );

    FloatPixel sum(sums.sumR, sums.sumG, sums.sumB);
    sum /= sums.count;
    return sum;
}



void scale_brightness(QImage& image, const FloatPixel& multiplier){
    Kernels::scale_brightness(
        image.width(), image.height(),
        (uint32_t*)image.bits(), image.bytesPerLine(),
        (float)multiplier.r, (float)multiplier.g, (float)multiplier.b
    );
}



double pixel_RMSD(const QImage& reference, const QImage& image){
    if (!image.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }
    if (reference.size() != image.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    uint64_t count = 0;
    uint64_t sumsqrs = 0;
    Kernels::sum_sqr_deviation(
        count, sumsqrs,
        reference.width(), reference.height(),
        (const uint32_t*)reference.bits(), reference.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine()
    );
    return std::sqrt((double)sumsqrs / (double)count);
}
double pixel_RMSD(const QImage& reference, const QImage& image, QRgb background){
    if (!image.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }
    if (reference.size() != image.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    uint64_t count = 0;
    uint64_t sumsqrs = 0;
    Kernels::sum_sqr_deviation(
        count, sumsqrs,
        reference.width(), reference.height(),
        (const uint32_t*)reference.bits(), reference.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        background
    );
    return std::sqrt((double)sumsqrs / (double)count);
}
double pixel_RMSD_masked(const QImage& reference, const QImage& image){
    if (!image.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }
    if (reference.size() != image.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    uint64_t count = 0;
    uint64_t sumsqrs = 0;
    Kernels::sum_sqr_deviation_masked(
        count, sumsqrs,
        reference.width(), reference.height(),
        (const uint32_t*)reference.bits(), reference.bytesPerLine(),
        (const uint32_t*)image.bits(), image.bytesPerLine()
    );
    return std::sqrt((double)sumsqrs / (double)count);
}



}
}
