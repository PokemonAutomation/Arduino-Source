/*  Image HSV (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Cpp/Exceptions.h"
#include "ImageHSV32.h"
#include "ImageViewHSV32.h"

namespace PokemonAutomation{



ImageHSV32 ImageViewHSV32::copy() const{
    if (m_ptr == nullptr){
        return ImageHSV32();
    }
    ImageHSV32 ret(m_width, m_height);
    if (ret.m_bytes_per_row == m_bytes_per_row){
        memcpy(
            ret.m_ptr, m_ptr,
            (m_height - 1) * m_bytes_per_row + m_width * sizeof(uint32_t)
        );
    }else{
        char* dst = (char*)ret.m_ptr;
        const char* src = (const char*)m_ptr;
        for (size_t c = 0; c < m_height; c++){
            memcpy(dst, src, m_width * sizeof(uint32_t));
            dst += ret.m_bytes_per_row;
            src += m_bytes_per_row;
        }
    }
    return ret;
}









}
