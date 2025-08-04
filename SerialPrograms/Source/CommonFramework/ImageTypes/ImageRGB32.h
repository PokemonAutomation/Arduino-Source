/*  Image (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageRGB32_H
#define PokemonAutomation_CommonFramework_ImageRGB32_H

#include <string>
#include "Common/Cpp/Containers/Pimpl.h"
#include "ImageViewRGB32.h"

namespace PokemonAutomation{


class ImageRGB32 : public ImageViewRGB32{
public:
    ~ImageRGB32();
    ImageRGB32(ImageRGB32&& x) noexcept;
    ImageRGB32& operator=(ImageRGB32&& x) noexcept;
private:
    //  Disable these to prevent implicit copying.
    ImageRGB32(const ImageRGB32& x) = delete;
    void operator=(const ImageRGB32& x) = delete;


public:
    ImageRGB32();
    //  Create an ARGB32 image of shape width x height with uninitialized pixels.
    ImageRGB32(size_t width, size_t height);
    explicit ImageRGB32(const std::string& filename);

    //  Fill the entire image with the specified pixel.
    using ImageViewPlanar32::fill;


public:
    //  Returns true if this image is valid. (non-null and non-zero dimensions)
    using ImageViewRGB32::operator bool;

    const uint32_t* data() const{ return m_ptr; }
          uint32_t* data(){ return m_ptr; }

    using ImageViewRGB32::bytes_per_row;
    using ImageViewRGB32::width;
    using ImageViewRGB32::height;

    //  Direct Pixel Access
    PA_FORCE_INLINE uint32_t pixel(size_t x, size_t y) const{
        return ImageViewPlanar32::pixel(x, y);
    }
    PA_FORCE_INLINE uint32_t& pixel(size_t x, size_t y){
        return ImageViewPlanar32::pixel(x, y);
    }

public:
    using ImageViewRGB32::sub_image;


public:
    using ImageViewRGB32::save;


public:
    //  QImage

    ImageRGB32(QImage image);
    using ImageViewRGB32::to_QImage_ref;
    using ImageViewRGB32::to_QImage_owning;
    using ImageViewRGB32::scaled_to_QImage;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
