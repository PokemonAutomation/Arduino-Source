/*  Image Reference (Planar 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "Planar32ImageView.h"

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






}
