/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "ImageFilter.h"

namespace PokemonAutomation{



size_t filter_rgb32_range(QImage& image, uint32_t mins, uint32_t maxs, Color replace_with, bool invert){
    if (!(image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)){
        image = image.convertToFormat(QImage::Format_RGB32);
    }
    size_t count;
    Kernels::filter_rgb32_range(
        (uint32_t*)image.bits(), image.bytesPerLine(), image.width(), image.height(),
        count, (uint32_t*)image.bits(), image.bytesPerLine(), mins, maxs, (uint32_t)replace_with, invert
    );
    return count;
}

void filter1_rgb32_range(
    const ConstImageRef& image,
    size_t& count0, QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0
){
    image0 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);

    Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        count0, (uint32_t*)image0.bits(), image0.bytesPerLine(), mins0, maxs0, (uint32_t)replace_with0, invert0
    );
}
void filter2_rgb32_range(
    const ConstImageRef& image,
    size_t& count0, QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0,
    size_t& count1, QImage& image1, uint32_t mins1, uint32_t maxs1, Color replace_with1, bool invert1
){
    image0 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
    image1 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);

    Kernels::filter2_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        count0, (uint32_t*)image0.bits(), image0.bytesPerLine(), mins0, maxs0, (uint32_t)replace_with0, invert0,
        count1, (uint32_t*)image1.bits(), image1.bytesPerLine(), mins1, maxs1, (uint32_t)replace_with1, invert1
    );
}
void filter4_rgb32_range(
    const ConstImageRef& image,
    size_t& count0, QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0,
    size_t& count1, QImage& image1, uint32_t mins1, uint32_t maxs1, Color replace_with1, bool invert1,
    size_t& count2, QImage& image2, uint32_t mins2, uint32_t maxs2, Color replace_with2, bool invert2,
    size_t& count3, QImage& image3, uint32_t mins3, uint32_t maxs3, Color replace_with3, bool invert3
){
    image0 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
    image1 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
    image2 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
    image3 = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);

    Kernels::filter4_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        count0, (uint32_t*)image0.bits(), image0.bytesPerLine(), mins0, maxs0, (uint32_t)replace_with0, invert0,
        count1, (uint32_t*)image1.bits(), image1.bytesPerLine(), mins1, maxs1, (uint32_t)replace_with1, invert1,
        count2, (uint32_t*)image2.bits(), image2.bytesPerLine(), mins2, maxs2, (uint32_t)replace_with2, invert2,
        count3, (uint32_t*)image3.bits(), image3.bytesPerLine(), mins3, maxs3, (uint32_t)replace_with3, invert3
    );
}



}
