/*  Image View (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <opencv2/core/mat.hpp>
//#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "ImageRGB32.h"
#include "ImageViewRGB32.h"

#ifdef QT_CORE_LIB
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include "ImageRGB32Qt.h"
#endif

namespace PokemonAutomation{




ImageRGB32 ImageViewRGB32::copy() const{
    if (m_ptr == nullptr){
        return ImageRGB32();
    }
    ImageRGB32 ret(m_width, m_height);
    ret.copy_from(*this);
    return ret;
}

#ifdef QT_CORE_LIB
bool ImageViewRGB32::save(const std::string& path) const{
    QString filepath = QString::fromStdString(path);
    QFileInfo fileInfo(filepath);
    QDir dir = fileInfo.dir();

    if (!dir.exists()){
        if (!dir.mkpath(".")){ // Create the path (the "." refers to the dir path itself)
            global_logger_tagged().log("Failed to create directory for saving image:" + dir.absolutePath().toStdString());
            return false;
        }
    }
    const bool success = to_QImage_ref(*this).save(QString::fromStdString(path));
    if (!success){
        global_logger_tagged().log("Failed to save image to:" + path);
    }
    return success;
}

ImageRGB32 ImageViewRGB32::scale_to(size_t width, size_t height) const{
    return QImage_to_ImageRGB32(scaled_to_QImage(*this, width, height));
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
