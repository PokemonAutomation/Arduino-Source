/*  Video Source (Camera)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/DeviceDropdownAliases.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "VideoSource_Camera.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



VideoSourceDescriptor_Camera::VideoSourceDescriptor_Camera(CameraInfo info)
    : VideoSourceDescriptor(VideoSourceType::Camera)
    , m_info(std::move(info))
{
    update_display_name();
}

void VideoSourceDescriptor_Camera::update_display_name(){
    if (!m_info){
        m_display_name.clear();
        return;
    }
    if (auto alias = device_dropdown_name("Cameras", m_info.device_name())){
        m_display_name = *alias;
    }else{
        m_display_name = get_camera_name(m_info);
    }
}

bool VideoSourceDescriptor_Camera::operator==(const VideoSourceDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_info == static_cast<const VideoSourceDescriptor_Camera&>(x).m_info;
}
std::string VideoSourceDescriptor_Camera::display_name() const{
    return m_display_name;
}

JsonValue VideoSourceDescriptor_Camera::to_json() const{
    return m_info.device_name();
}
void VideoSourceDescriptor_Camera::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name != nullptr){
        m_info = CameraInfo(*name);
        update_display_name();
    }
}

std::unique_ptr<VideoSource> VideoSourceDescriptor_Camera::make_VideoSource(
    Logger& logger,
    Resolution resolution,
    VideoFormat format,
    FramesPerSecond fps
) const{
//    cout << "format = " << (int)format << endl;
    return get_camera_backend().make_video_source(
        logger,
        m_info,
        resolution,
        format,
        fps
    );
}




}


