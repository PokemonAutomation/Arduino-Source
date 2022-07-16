/*  Image Reference (Planar 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageViewPlanar32_H
#define PokemonAutomation_CommonFramework_ImageViewPlanar32_H

#include <stdint.h>

namespace PokemonAutomation{



class ImageViewPlanar32{
public:
    ImageViewPlanar32() = default;
    ImageViewPlanar32(uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height)
        : m_width(width), m_height(height)
        , m_bytes_per_row(bytes_per_row), m_ptr(ptr)
    {}


public:
    operator bool() const{ return m_ptr != nullptr; }

    uint32_t*       data            () const{ return m_ptr; }
    size_t          bytes_per_row   () const{ return m_bytes_per_row; }
    size_t          width           () const{ return m_width; }
    size_t          height          () const{ return m_height; }

    uint32_t pixel(size_t x, size_t y) const{
        return *(const uint32_t*)((const char*)m_ptr + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }

    ImageViewPlanar32 sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const;


protected:
    uint32_t& get_pixel(size_t x, size_t y) const{
        return *(uint32_t*)((char*)m_ptr + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }


protected:
    size_t m_width = 0;
    size_t m_height = 0;
    size_t m_bytes_per_row = 0;
    uint32_t* m_ptr = nullptr;
};





}
#endif
