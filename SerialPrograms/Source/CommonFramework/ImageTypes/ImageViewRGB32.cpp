/*  Image View (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Filesystem.h"
#include "CommonFramework/Logging/Logger.h"
#include "ImageRGB32.h"
#include "ImageViewRGB32.h"

//#define PA_IMAGE_BACKEND_Qt
#define PA_IMAGE_BACKEND_OpenCV

#if 0
#elif defined PA_IMAGE_BACKEND_Qt
#include "ImageRGB32_Qt.h"
#elif defined PA_IMAGE_BACKEND_OpenCV
#include "ImageRGB32_OpenCV.h"
#else
#error "No image backend specified."
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

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
    Filesystem::Path folder = Filesystem::Path(path).parent_path();

    try{
        Filesystem::create_directories(folder);
    }catch (...){}

    if (!folder.empty() && !Filesystem::exists(folder)){
        global_logger_tagged().log("Failed to create directory for saving image:" + folder.string());
        return false;
    }

#ifdef PA_IMAGE_BACKEND_Qt
    return QImage_save_image(*this, path);
#endif
#ifdef PA_IMAGE_BACKEND_OpenCV
    return OpenCV_save_image(*this, path);
}

#endif


#ifdef PA_IMAGE_BACKEND_Qt
ImageRGB32 ImageViewRGB32::scale_to(size_t width, size_t height) const{
    return QImage_to_ImageRGB32(scaled_to_QImage(*this, width, height));
}
#endif
#ifdef PA_IMAGE_BACKEND_OpenCV
ImageRGB32 ImageViewRGB32::scale_to(size_t width, size_t height) const{
    return OpenCV_scale_image(*this, width, height);
}
#endif



cv::Mat ImageViewRGB32::to_opencv_Mat() const{
    return cv::Mat{
        static_cast<int>(m_height),
        static_cast<int>(m_width),
        CV_8UC4,
        (cv::Scalar*)m_ptr,
        m_bytes_per_row
    };
}






}
