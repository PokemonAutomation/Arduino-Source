/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraImplementations_H
#define PokemonAutomation_VideoPipeline_CameraImplementations_H

#include <functional>
#include <vector>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CameraInfo.h"
#include "VideoWidget.h"

namespace PokemonAutomation{


class VideoBackendOption : public EnumDropdownOption{
public:
    VideoBackendOption();
};



class CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const = 0;
    virtual std::string get_camera_name(const CameraInfo& info) const = 0;

    virtual std::unique_ptr<Camera> make_camera(
        Logger& logger,
        const CameraInfo& info,
        const Resolution& desired_resolution
    ) const = 0;

    virtual VideoWidget* make_video_widget(QWidget* parent, Camera& camera) const = 0;

    virtual VideoWidget* make_video_widget(
        QWidget& parent,
        Logger& logger,
        const CameraInfo& info,
        const Resolution& desired_resolution
    ) const = 0;

    virtual ~CameraBackend() {}
};



std::vector<CameraInfo> get_all_cameras();
std::string get_camera_name(const CameraInfo& info);

const CameraBackend& get_camera_backend();

std::unique_ptr<Camera> make_camera(
    Logger& logger,
    const CameraInfo& info, const Resolution& desired_resolution
);
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    Logger& logger,
    const CameraInfo& info, const Resolution& desired_resolution
);




}
#endif
