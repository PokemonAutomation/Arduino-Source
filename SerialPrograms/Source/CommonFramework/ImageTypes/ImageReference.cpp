/*  Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "ImageReference.h"

namespace PokemonAutomation{



ImageRef::ImageRef()
    : m_data(nullptr)
    , m_bytes_per_row(0)
    , m_width(0)
    , m_height(0)
{}
ImageRef::ImageRef(uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height)
    : m_data(ptr), m_bytes_per_row(bytes_per_row)
    , m_width(width)
    , m_height(height)
{}
ImageRef::ImageRef(QImage& image){
    if (image.isNull()){
        m_data = nullptr;
        m_bytes_per_row = 0;
        m_width = 0;
        m_height = 0;
    }else{
        QImage::Format format = image.format();
        if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format.");
        }
        m_data = (uint32_t*)image.bits();
        m_bytes_per_row = image.bytesPerLine();
        m_width = image.width();
        m_height = image.height();
    }
}
ImageRef ImageRef::sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
    if (min_x >= m_width || min_y >= m_height){
        return ImageRef();
    }
    width = std::min(width, m_width - min_x);
    height = std::min(height, m_height - min_y);
    return ImageRef(
        (uint32_t*)((char*)m_data + min_y * m_bytes_per_row) + min_x,
        m_bytes_per_row,
        width, height
    );
}
void ImageRef::save(const QString& path) const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    tmp.save(path);
}
QImage ImageRef::to_qimage() const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    return tmp.copy();
}
QImage ImageRef::scaled_to_qimage(size_t width, size_t height) const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    if (m_width == width && m_height == height){
        return tmp.copy();
    }
    return tmp.scaled((int)width, (int)height);
}





ConstImageRef::ConstImageRef()
    : m_data(nullptr)
    , m_bytes_per_row(0)
    , m_width(0)
    , m_height(0)
{}
ConstImageRef::ConstImageRef(const uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height)
    : m_data(ptr), m_bytes_per_row(bytes_per_row)
    , m_width(width)
    , m_height(height)
{}
ConstImageRef::ConstImageRef(const QImage& image){
    if (image.isNull()){
        m_data = nullptr;
        m_bytes_per_row = 0;
        m_width = 0;
        m_height = 0;
    }else{
        QImage::Format format = image.format();
        if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format.");
        }
        m_data = (const uint32_t*)image.bits();
        m_bytes_per_row = image.bytesPerLine();
        m_width = image.width();
        m_height = image.height();
    }
}
ConstImageRef ConstImageRef::sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
    if (min_x >= m_width || min_y >= m_height){
        return ConstImageRef();
    }
    width = std::min(width, m_width - min_x);
    height = std::min(height, m_height - min_y);
    return ConstImageRef(
        (const uint32_t*)((const char*)m_data + min_y * m_bytes_per_row) + min_x,
        m_bytes_per_row,
        width, height
    );
}
void ConstImageRef::save(const QString& path) const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    tmp.save(path);
}
QImage ConstImageRef::to_qimage() const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    return tmp.copy();
}
QImage ConstImageRef::scaled_to_qimage(size_t width, size_t height) const{
    QImage tmp((const uchar*)m_data, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    if (m_width == width && m_height == height){
        return tmp.copy();
    }
    return tmp.scaled((int)width, (int)height);
}










}
