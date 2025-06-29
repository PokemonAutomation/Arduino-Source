/*  Camera Video Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Backends/CameraImplementations.h"
#include "VideoSource.h"
#include "VideoSource_Camera.h"

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

std::unique_ptr<VideoSource> VideoSourceDescriptor_Camera::make_VideoSource(Logger& logger, Resolution resolution) const{
    return get_camera_backend().make_video_source(logger, m_info, resolution);
}




}


