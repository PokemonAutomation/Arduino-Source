/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/BinaryImage/Kernels_BinaryImage.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "BinaryImage.h"

namespace PokemonAutomation{

class BinaryImageImpl : public Kernels::BinaryImage{
public:
    using Kernels::BinaryImage::BinaryImage;
};


BinaryImage::~BinaryImage(){}
BinaryImage::BinaryImage(size_t width, size_t height)
    : m_ptr(new BinaryImageImpl(width, height))
{}

size_t BinaryImage::width() const{
    return m_ptr->width();
}
size_t BinaryImage::height() const{
    return m_ptr->height();
}

void BinaryImage::set_zero(){
    m_ptr->set_zero();
}

bool BinaryImage::get_pixel(size_t x, size_t y) const{
    return m_ptr->pixel(x, y);
}
void BinaryImage::set_pixel(size_t x, size_t y, bool set){
    m_ptr->set_pixel(x, y, set);
}
std::string BinaryImage::dump() const{
    return m_ptr->dump();
}





BinaryImage filter_min_rgb32(
    const QImage& image,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
    BinaryImage ret(image.width(), image.height());
    Kernels::filter_min_rgb32(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        min_alpha, min_red, min_green, min_blue
    );
    return ret;
}
BinaryImage filter_rgb32_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    BinaryImage ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
    return ret;
}




}
