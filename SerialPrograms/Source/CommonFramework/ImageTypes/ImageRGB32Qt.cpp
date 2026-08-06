/*  Image (RGB32) Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ImageRGB32Qt.h"

namespace PokemonAutomation{



QImage to_QImage_ref(const ImageViewRGB32& image){
    return QImage(
        (const uchar*)image.data(),
        (int)image.width(),
        (int)image.height(),
        (int)image.bytes_per_row(),
        QImage::Format_ARGB32
    );
}
QImage to_QImage_owning(const ImageViewRGB32& image){
    return to_QImage_ref(image).copy();
}
QImage scaled_to_QImage(const ImageViewRGB32& image, size_t width, size_t height){
    QImage tmp(
        (const uchar*)image.data(),
        (int)image.width(),
        (int)image.height(),
        (int)image.bytes_per_row(),
        QImage::Format_ARGB32
    );
    if (image.width() == width && image.height() == height){
        return tmp.copy();
    }
    return tmp.scaled((int)width, (int)height);
//    return tmp.scaled((int)width, (int)height, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
}


ImageRGB32 QImage_to_ImageRGB32(QImage image){
    return ImageRGB32(std::make_unique<ImageRGB32Qt>(std::move(image)));
}


ImageRGB32Qt::ImageRGB32Qt(QImage image)
    : m_image(std::move(image))
{
    if (m_image.isNull()){
        return;
    }
    QImage::Format format = m_image.format();
    if (format == QImage::Format_ARGB32_Premultiplied){
        m_image = m_image.convertToFormat(QImage::Format_ARGB32);
    }else if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        std::cout << "Non standard QImage format: " + std::to_string((int)format) << std::endl;
        // image = image.convertToFormat(QImage::Format_ARGB32);
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format: " + std::to_string((int)format));
    }
}

ImageViewRGB32 ImageRGB32Qt::get_view() const{
    if (m_image.isNull()){
        return ImageViewRGB32();
    }
    return ImageViewRGB32(
        (uint32_t*)m_image.bits(),
        m_image.bytesPerLine(),
        m_image.width(),
        m_image.height()
    );
}






}
