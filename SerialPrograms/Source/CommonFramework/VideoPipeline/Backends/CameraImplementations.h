/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraImplementations_H
#define PokemonAutomation_VideoPipeline_CameraImplementations_H

#include <functional>
#include <vector>
#include "Common/Cpp/Options/DropdownOption.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/VideoPipeline/UI/VideoWidget.h"

namespace PokemonAutomation{


class VideoBackendOption : public DropdownOption{
public:
    VideoBackendOption();
};



class CameraBackend{
public:
    virtual ~CameraBackend() = default;

    virtual std::vector<CameraInfo> get_all_cameras() const = 0;
    virtual std::string get_camera_name(const CameraInfo& info) const = 0;

    virtual std::unique_ptr<CameraSession> make_camera(Logger& logger, Resolution default_resolution) const = 0;
};



std::vector<CameraInfo> get_all_cameras();
std::string get_camera_name(const CameraInfo& info);

const CameraBackend& get_camera_backend();





}
#endif
