/*  Image (RGB32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include "CompileTimeBackends.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "ImageViewRGB32.h"
#include "ImageRGB32.h"

#ifdef PA_IMAGE_BACKEND_Qt
#include "ImageRGB32_Qt.h"
#endif
#ifdef PA_IMAGE_BACKEND_OpenCV
#include "ImageRGB32_OpenCV.h"
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

#ifdef PA_IMAGE_BACKEND_Qt
ImageRGB32::ImageRGB32(const std::string& filename){
    *this = QImage_load_image(filename);
}
#endif
#ifdef PA_IMAGE_BACKEND_OpenCV
ImageRGB32::ImageRGB32(const std::string& filename){
    *this = OpenCV_load_image(filename);
}
#endif

ImageRGB32::ImageRGB32(std::unique_ptr<CustomImageRGB32Owner> image)
    : m_custom_owner(std::move(image))
{
    ImageViewRGB32::operator=(m_custom_owner->get_view());
}






}
