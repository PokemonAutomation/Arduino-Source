/*  Image (RGB32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "ImageViewRGB32.h"
#include "ImageRGB32.h"

#ifdef QT_CORE_LIB
#include <QImage>
#include "ImageRGB32Qt.h"
#endif

namespace PokemonAutomation{



ImageRGB32::~ImageRGB32() = default;
ImageRGB32::ImageRGB32(ImageRGB32&& x) noexcept{
    *this = std::move(x);
}
ImageRGB32& ImageRGB32::operator=(ImageRGB32&& x) noexcept{
    if (this != &x){
        ImageViewRGB32::operator=(x);
        m_data = std::move(x.m_data);
        m_custom_owner = std::move(x.m_custom_owner);
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
    , m_data(m_bytes_per_row / sizeof(uint32_t) * height)
{
    m_ptr = m_data.data();
}

#ifdef QT_CORE_LIB
ImageRGB32::ImageRGB32(const std::string& filename){
    QImage image(QString::fromStdString(filename));
    if (image.isNull()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to open image.", filename);
    }
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    *this = ImageRGB32(std::make_unique<ImageRGB32Qt>(std::move(image)));
}
#endif

ImageRGB32::ImageRGB32(std::unique_ptr<CustomImageRGB32Owner> image)
    : m_custom_owner(std::move(image))
{
    ImageViewRGB32::operator=(m_custom_owner->get_view());
}






}
