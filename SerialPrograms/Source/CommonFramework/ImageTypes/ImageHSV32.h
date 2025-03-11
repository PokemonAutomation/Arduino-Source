/*  Image (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageHSV32_H
#define PokemonAutomation_CommonFramework_ImageHSV32_H

#include <string>
#include "Common/Cpp/Containers/Pimpl.h"
#include "ImageViewHSV32.h"

namespace PokemonAutomation{

class ImageViewRGB32;


class ImageHSV32 : public ImageViewHSV32{
public:
public:
    ~ImageHSV32();
    ImageHSV32(ImageHSV32&& x) noexcept;
    ImageHSV32& operator=(ImageHSV32&& x) noexcept;
private:
    //  Disable these to prevent implicit copying.
    ImageHSV32(const ImageHSV32& x) = delete;
    void operator=(const ImageHSV32& x) = delete;


public:
    ImageHSV32();
    ImageHSV32(size_t width, size_t height);

    //  Fill the entire image with the specified pixel.
    using ImageViewPlanar32::fill;


public:
    //  Returns true if this image is valid. (non-null and non-zero dimensions)
    using ImageViewHSV32::operator bool;

    const uint32_t* data() const{ return m_ptr; }
          uint32_t* data(){ return m_ptr; }

    using ImageViewHSV32::bytes_per_row;
    using ImageViewHSV32::width;
    using ImageViewHSV32::height;

    //  Direct Pixel Access
    PA_FORCE_INLINE uint32_t pixel(size_t x, size_t y) const{
        return ImageViewPlanar32::pixel(x, y);
    }
    PA_FORCE_INLINE uint32_t& pixel(size_t x, size_t y){
        return ImageViewPlanar32::pixel(x, y);
    }

public:
    using ImageViewHSV32::sub_image;


public:
    //  HSV32

    explicit ImageHSV32(const ImageViewRGB32& image);


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
