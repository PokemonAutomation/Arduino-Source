/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "CameraImplementations.h"

#if QT_VERSION_MAJOR == 5
#include "CameraWidgetQt5.h"
#include "CameraWidgetQt5v2.h"
#elif QT_VERSION_MAJOR == 6
#include "CameraWidgetQt6.h"
#endif


namespace PokemonAutomation{


std::vector<CameraInfo> get_all_cameras(){
#if QT_VERSION_MAJOR == 5
    return CameraQt5::qt5_get_all_cameras();
#elif QT_VERSION_MAJOR == 6
    return CameraQt6::qt6_get_all_cameras();
#endif
}
QString get_camera_name(const CameraInfo& info){
#if QT_VERSION_MAJOR == 5
    return CameraQt5::qt5_get_camera_name(info);
#elif QT_VERSION_MAJOR == 6
    return CameraQt6::qt6_get_camera_name(info);
#endif
}
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
){
    return [&](QWidget& parent){
#if QT_VERSION_MAJOR == 5
        return new CameraQt5::Qt5VideoWidget2(&parent, logger, info, desired_resolution);
#elif QT_VERSION_MAJOR == 6
        return new CameraQt6::Qt6VideoWidget(&parent, logger, info, desired_resolution);
#endif
    };
}


}
