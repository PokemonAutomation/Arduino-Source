/*  Image Histogram
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageHistogram_H
#define PokemonAutomation_CommonFramework_ImageHistogram_H

#include <array>
#include <cstdint>

namespace PokemonAutomation{
    class ImageViewRGB32;


// A histogram for the distribution of the brightness on an image.
// Brightness is defined here as max(R, G, B) of a pixel, which in HSV is called "value".
// This histogram is useful as a feature of an image or to compute a cutoff threshold
// for binarization of the image.
class BrightestChannelHistogram{
public:
    // Add a pixel (0xAARRGGBB) into the histogram. Its alpha is ignored.
    void add(uint32_t pixel);

    // How many pixels were added
    uint64_t count() const{ return m_count; }

    // Return the percentile. 0 if no pixel were added.
    // Given a `fraction` (0.0 to 1.0), it sorts the counted pixels from darkest to brightnest
    // and return the brightness level so that the pixels having brightness smaller or equal
    // to that takes up this fraction of the total pixel count.
    // e.g. 0.5 gives the median and 1.0 gives the max brightness recorded.
    uint8_t percentile(double fraction) const;

private:
    std::array<uint64_t, 256> m_bins{};
    uint64_t m_count = 0;
};

//  Pixels with alpha < 128 are ignored.
BrightestChannelHistogram image_brightest_channel_histogram(const ImageViewRGB32& image);

// Same as `image_brightest_channel_histogram()`, but only on the one-pixel-wide
// border of the image, with each border pixel counted once.
BrightestChannelHistogram image_border_brightest_channel_histogram(const ImageViewRGB32& image);



}
#endif
