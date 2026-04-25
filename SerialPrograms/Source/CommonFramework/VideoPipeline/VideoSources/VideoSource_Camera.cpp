/*  Video Source (Camera)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "VideoSource_Camera.h"

#ifdef _WIN32
#include "CommonFramework/VideoPipeline/Backends/DirectShowCameraList.h"
#include "CommonFramework/VideoPipeline/Backends/CameraVideoSource_OpenCV.h"
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



bool VideoSourceDescriptor_Camera::operator==(const VideoSourceDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_info == static_cast<const VideoSourceDescriptor_Camera&>(x).m_info;
}
std::string VideoSourceDescriptor_Camera::display_name() const{
    return get_camera_name(m_info);
}

void VideoSourceDescriptor_Camera::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name != nullptr){
        m_info = CameraInfo(*name);
    }
}
JsonValue VideoSourceDescriptor_Camera::to_json() const{
    return m_info.device_name();
}

std::unique_ptr<VideoSource> VideoSourceDescriptor_Camera::make_VideoSource(
    Logger& logger,
    Resolution resolution
) const{
#ifdef _WIN32
    if (is_directshow_device(m_info)){
        int index = get_directshow_device_index(m_info);
        std::string name = get_directshow_display_name(m_info);
        return std::make_unique<CameraVideoSource_OpenCV>(logger, index, name, resolution);
    }
#endif
    return get_camera_backend().make_video_source(logger, m_info, resolution);
}




}


