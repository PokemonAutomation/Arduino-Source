/*  Image View (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QImage>
#include <opencv2/core/mat.hpp>
#include "Common/Cpp/Exceptions.h"
#include "ImageRGB32.h"
#include "ImageViewRGB32.h"

namespace PokemonAutomation{




ImageRGB32 ImageViewRGB32::copy() const{
    if (m_ptr == nullptr){
        return ImageRGB32();
    }
    ImageRGB32 ret(m_width, m_height);
    ret.copy_from(*this);
    return ret;
}
bool ImageViewRGB32::save(const std::string& path) const{
    return to_QImage_ref().save(QString::fromStdString(path));
}
ImageRGB32 ImageViewRGB32::scale_to(size_t width, size_t height) const{
    return scaled_to_QImage(width, height);
}



ImageViewRGB32::ImageViewRGB32(const QImage& image){
    if (image.isNull()){
        return;
    }
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format.");
    }
    m_width = image.width();
    m_height = image.height();
    m_bytes_per_row = image.bytesPerLine();
    m_ptr = (uint32_t*)image.bits();    //  Intentionally casting away const. It won't be modified.
}
QImage ImageViewRGB32::to_QImage_ref() const{
    return QImage((const uchar*)m_ptr, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
}
QImage ImageViewRGB32::to_QImage_owning() const{
    return to_QImage_ref().copy();
}
QImage ImageViewRGB32::scaled_to_QImage(size_t width, size_t height) const{
    QImage tmp((const uchar*)m_ptr, (int)m_width, (int)m_height, (int)m_bytes_per_row, QImage::Format_ARGB32);
    if (m_width == width && m_height == height){
        return tmp.copy();
    }
    return tmp.scaled((int)width, (int)height);
//    return tmp.scaled((int)width, (int)height, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
}
cv::Mat ImageViewRGB32::to_opencv_Mat() const{
    return cv::Mat{ static_cast<int>(m_height), static_cast<int>(m_width), CV_8UC4, (cv::Scalar*)m_ptr, m_bytes_per_row };
}






}
