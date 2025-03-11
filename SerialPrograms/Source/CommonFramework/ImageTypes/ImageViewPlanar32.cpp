/*  Image Reference (Planar 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <algorithm>
#include "ImageViewPlanar32.h"

namespace PokemonAutomation{



ImageViewPlanar32 ImageViewPlanar32::sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
    if (min_x >= m_width || min_y >= m_height){
        return ImageViewPlanar32();
    }
    width = std::min(width, m_width - min_x);
    height = std::min(height, m_height - min_y);
    return ImageViewPlanar32(
        (uint32_t*)((char*)m_ptr + min_y * m_bytes_per_row) + min_x,
        m_bytes_per_row,
        width, height
    );
}


void ImageViewPlanar32::fill(uint32_t pixel){
    size_t width = m_width;
    size_t height = m_height;
    uint32_t* dst = m_ptr;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            dst[c] = pixel;
        }
        dst = (uint32_t*)((char*)dst + m_bytes_per_row);
    }
}
void ImageViewPlanar32::copy_from(const ImageViewPlanar32& source){
    if (m_bytes_per_row == source.m_bytes_per_row){
        memcpy(
            m_ptr, source.m_ptr,
            (m_height - 1) * m_bytes_per_row + m_width * sizeof(uint32_t)
        );
    }else{
        char* dst = (char*)m_ptr;
        const char* src = (const char*)source.m_ptr;
        for (size_t c = 0; c < m_height; c++){
            memcpy(dst, src, m_width * sizeof(uint32_t));
            dst += m_bytes_per_row;
            src += source.m_bytes_per_row;
        }
    }
}





}
