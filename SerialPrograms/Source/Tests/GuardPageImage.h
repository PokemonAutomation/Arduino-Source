/*  Guard Page Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  An RGB32 image for unit tests of image kernels. The rows are packed with
 *  no padding (bytes_per_row = width * 4), and the last row ends right before
 *  a no-access guard page. A kernel that reads or writes past the last pixel
 *  faults right away.
 *
 *  ImageRGB32 pads every row to 64 bytes, so tests on it never notice a kernel
 *  that reads a little past the end of a row. Views of other memory (like a
 *  QImage frame) don't have that padding.
 *
 */

#ifndef PokemonAutomation_Tests_GuardPageImage_H
#define PokemonAutomation_Tests_GuardPageImage_H

#include <stdint.h>
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{


class GuardPageImageRGB32{
public:
    //  A blank image. Every pixel starts as zero.
    GuardPageImageRGB32(size_t width, size_t height)
        : m_width(width)
        , m_height(height)
        , m_pixels(width * height)
    {}

    //  A copy of "image".
    explicit GuardPageImageRGB32(const ImageViewRGB32& image)
        : GuardPageImageRGB32(image.width(), image.height())
    {
        for (size_t y = 0; y < m_height; y++){
            for (size_t x = 0; x < m_width; x++){
                m_pixels[x + y * m_width] = image.pixel(x, y);
            }
        }
    }

    size_t width() const{ return m_width; }
    size_t height() const{ return m_height; }
    size_t bytes_per_row() const{ return m_width * sizeof(uint32_t); }

    const uint32_t* data() const{ return m_pixels.data(); }
          uint32_t* data()      { return m_pixels.data(); }

    uint32_t pixel(size_t x, size_t y) const{
        return m_pixels[x + y * m_width];
    }

private:
    size_t m_width;
    size_t m_height;
    GuardPageBuffer<uint32_t> m_pixels;
};



}
#endif
