/*  Image Reference (Planar 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageViewPlanar32_H
#define PokemonAutomation_CommonFramework_ImageViewPlanar32_H

#include <stdint.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{



class ImageViewPlanar32{
public:
    PA_FORCE_INLINE ImageViewPlanar32() = default;
    PA_FORCE_INLINE ImageViewPlanar32(uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height)
        : m_width(width), m_height(height)
        , m_bytes_per_row(bytes_per_row), m_ptr(ptr)
    {}


public:
    PA_FORCE_INLINE operator bool() const{ return m_ptr != nullptr; }

    PA_FORCE_INLINE const uint32_t* data            () const{ return m_ptr; }
    PA_FORCE_INLINE size_t          bytes_per_row   () const{ return m_bytes_per_row; }
    PA_FORCE_INLINE size_t          width           () const{ return m_width; }
    PA_FORCE_INLINE size_t          height          () const{ return m_height; }

    PA_FORCE_INLINE uint32_t pixel(size_t x, size_t y) const{
        return *(const uint32_t*)((const char*)m_ptr + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }

    ImageViewPlanar32 sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const;


protected:
    //  Helpers for child classes that allow modifications.
    PA_FORCE_INLINE uint32_t& pixel(size_t x, size_t y){
        return *(uint32_t*)((char*)m_ptr + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }
    void fill(uint32_t pixel);
    void copy_from(const ImageViewPlanar32& source);


protected:
    size_t m_width = 0;
    size_t m_height = 0;
    size_t m_bytes_per_row = 0;
    uint32_t* m_ptr = nullptr;
};





}
#endif
