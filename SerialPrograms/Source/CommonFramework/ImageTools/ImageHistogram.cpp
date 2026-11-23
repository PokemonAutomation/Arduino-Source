/*  Image Histogram
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

#include "ImageHistogram.h"

namespace PokemonAutomation{



void BrightestChannelHistogram::add(uint32_t pixel){
    const uint32_t red = (pixel >> 16) & 0xff;
    const uint32_t green = (pixel >> 8) & 0xff;
    const uint32_t blue = pixel & 0xff;
    m_bins[std::max({red, green, blue})]++;
    m_count++;
}
uint8_t BrightestChannelHistogram::percentile(double fraction) const{
    if (m_count == 0){
        return 0;
    }
    //  The position of the pixel to return among the sorted pixels, counted from 0.
    fraction = std::clamp(fraction, 0.0, 1.0);
    const uint64_t rank = std::min((uint64_t)((double)m_count * fraction), m_count - 1);
    uint64_t seen = 0;
    for (size_t level = 0; level < m_bins.size(); level++){
        seen += m_bins[level];
        if (seen > rank){
            return (uint8_t)level;
        }
    }
    return 255;
}

static void add_if_opaque(BrightestChannelHistogram& histogram, uint32_t pixel){
    if ((pixel >> 24) >= 128){
        histogram.add(pixel);
    }
}
BrightestChannelHistogram image_brightest_channel_histogram(const ImageViewRGB32& image){
    BrightestChannelHistogram histogram;
    const size_t width = image.width();
    const size_t height = image.height();
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            add_if_opaque(histogram, image.pixel(x, y));
        }
    }
    return histogram;
}
BrightestChannelHistogram image_border_brightest_channel_histogram(const ImageViewRGB32& image){
    BrightestChannelHistogram histogram;
    const size_t width = image.width();
    const size_t height = image.height();
    if (width == 0 || height == 0){
        return histogram;
    }
    //  Top and bottom rows, then the left and right columns without the corners.
    for (size_t x = 0; x < width; x++){
        add_if_opaque(histogram, image.pixel(x, 0));
        if (height > 1){
            add_if_opaque(histogram, image.pixel(x, height - 1));
        }
    }
    for (size_t y = 1; y + 1 < height; y++){
        add_if_opaque(histogram, image.pixel(0, y));
        if (width > 1){
            add_if_opaque(histogram, image.pixel(width - 1, y));
        }
    }
    return histogram;
}



}
