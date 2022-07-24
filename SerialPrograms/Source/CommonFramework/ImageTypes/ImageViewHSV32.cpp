/*  Image HSV (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "ImageReference.h"
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
bool ImageViewHSV32::save(const std::string& path) const{
    return to_QImage_ref().save(QString::fromStdString(path));
}



ImageViewHSV32::ImageViewHSV32(const QImage& image){
    if (image.isNull()){
        return;
    }
    QImage::Format format = image.format();
    if (format != QImage::Format_AHSV32 && format != QImage::Format_HSV32){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format.");
    }
    m_width = image.width();
    m_height = image.height();
    m_bytes_per_row = image.bytesPerLine();
    m_ptr = (uint32_t*)image.bits();    //  Intentionally casting away const. It won't be modified.
}
QImage ImageViewHSV32::to_QImage_ref() const{
    return QImage((const uchar*)m_ptr, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_AHSV32);
}
QImage ImageViewHSV32::to_QImage_owning() const{
    return to_QImage_ref().copy();
}






}
