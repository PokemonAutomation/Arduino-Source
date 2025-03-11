/*  Image (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include <QImage>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "ImageViewRGB32.h"
#include "ImageRGB32.h"

#include <iostream>


namespace PokemonAutomation{

struct ImageRGB32::Data{
    AlignedVector<uint32_t> self;
    QImage qimage;

    Data(size_t items) : self(items) {}
    Data(QImage image) : qimage(std::move(image)) {}
};



ImageRGB32::~ImageRGB32() = default;
ImageRGB32::ImageRGB32(ImageRGB32&& x) noexcept{
    *this = std::move(x);
}
ImageRGB32& ImageRGB32::operator=(ImageRGB32&& x) noexcept{
    if (this != &x){
        ImageViewRGB32::operator=(x);
        m_data = std::move(x.m_data);
        x.m_bytes_per_row = 0;
        x.m_ptr = nullptr;
        x.m_width = 0;
        x.m_height = 0;
    }
    return *this;
}
#if 0
ImageRGB32::ImageRGB32(const ImageRGB32& x){
    *this = copy();
}
void ImageRGB32::operator=(const ImageRGB32& x){
    *this = copy();
}
#endif

ImageRGB32::ImageRGB32() = default;

ImageRGB32::ImageRGB32(size_t width, size_t height)
    : ImageViewRGB32(width, height)
    , m_data(CONSTRUCT_TOKEN, m_bytes_per_row / sizeof(uint32_t) * height)
{
    m_ptr = m_data->self.data();
}
ImageRGB32::ImageRGB32(const std::string& filename){
    QImage image(QString::fromStdString(filename));
    if (image.isNull()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to open image.", filename);
    }
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    *this = std::move(image);
}




ImageRGB32::ImageRGB32(QImage image){
    if (image.isNull()){
        return;
    }
    QImage::Format format = image.format();
    if (format == QImage::Format_ARGB32_Premultiplied){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }else if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        std::cout << "Non standard QImage format: " + std::to_string((int)format) << std::endl;
        // image = image.convertToFormat(QImage::Format_ARGB32);
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid QImage format: " + std::to_string((int)format));
    }
    m_width = image.width();
    m_height = image.height();
    m_bytes_per_row = image.bytesPerLine();
    m_ptr = (uint32_t*)image.bits();
    m_data.reset(std::move(image));
}



}
