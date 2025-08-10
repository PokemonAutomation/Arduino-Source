/*  Image View (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageViewRGB32_H
#define PokemonAutomation_CommonFramework_ImageViewRGB32_H

#include <string>
#include "ImageViewPlanar32.h"

class QImage;
namespace cv{
    class Mat;
}

namespace PokemonAutomation{


class ImageRGB32;


class ImageViewRGB32 : public ImageViewPlanar32{
public:
    using ImageViewPlanar32::ImageViewPlanar32;

public:
    //  Returns true if this image is valid. (non-null and non-zero dimensions)
    using ImageViewPlanar32::operator bool;

    using ImageViewPlanar32::data;
    using ImageViewPlanar32::bytes_per_row;
    using ImageViewPlanar32::width;
    using ImageViewPlanar32::height;
    using ImageViewPlanar32::total_pixels;

    //  Direct Pixel Access
    PA_FORCE_INLINE uint32_t pixel(size_t x, size_t y) const{
        return ImageViewPlanar32::pixel(x, y);
    }

    PA_FORCE_INLINE ImageViewRGB32 sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
        return ImageViewPlanar32::sub_image(min_x, min_y, width, height);
    }

public:
    ImageRGB32 copy() const;
    bool save(const std::string& path) const;
    ImageRGB32 scale_to(size_t width, size_t height) const;

public:
    //  QImage

    ImageViewRGB32(const QImage& image);
    QImage to_QImage_ref() const;       //  Return a shallow copy-on-write reference that points to this buffer. (fast)
    QImage to_QImage_owning() const;    //  Return a copy that owns its own buffer. (slow)
    QImage scaled_to_QImage(size_t width, size_t height) const;

    // convert to cv::Mat with BGRA color channel order
    cv::Mat to_opencv_Mat() const;

private:
    PA_FORCE_INLINE ImageViewRGB32(const ImageViewPlanar32& x)
        : ImageViewPlanar32(x)
    {}
};





}
#endif
