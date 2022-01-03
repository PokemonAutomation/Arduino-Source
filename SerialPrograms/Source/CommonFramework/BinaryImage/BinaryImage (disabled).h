/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Wrap the entire Kernels::BinaryImage type so we're not
 *  littering the header with a zillion SIMD intrinsics.
 *
 *  This class is not meant to be used directly as the abstraction makes it very
 *  inefficient. It's mostly just a handle to be passed between compute-kernels.
 *
 */

#ifndef PokemonAutomation_CommonFramework_BinaryImage_BinaryImage_H
#define PokemonAutomation_CommonFramework_BinaryImage_BinaryImage_H

#include <memory>
#include <string>
#include <QImage>
#include "Common/Cpp/Pimpl.h"

namespace PokemonAutomation{

class BinaryImageImpl;


class BinaryImage{
public:
    ~BinaryImage();
    BinaryImage(BinaryImage&&) = default;
    BinaryImage& operator=(BinaryImage&&) = default;

public:
    BinaryImage(size_t width, size_t height);

    operator BinaryImageImpl&(){ return *m_ptr; }

    size_t width() const;
    size_t height() const;

    void set_zero();

    bool get_pixel(size_t x, size_t y) const;
    void set_pixel(size_t x, size_t y, bool set);

    std::string dump() const;


private:
    std::unique_ptr<BinaryImageImpl> m_ptr;
};




BinaryImage filter_min_rgb32(
    const QImage& image,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
);
BinaryImage filter_rgb32_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);





}
#endif
