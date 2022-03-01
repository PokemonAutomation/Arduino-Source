/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "CameraImplementations.h"

#if QT_VERSION_MAJOR == 5
#include "CameraWidgetQt5.h"
#elif QT_VERSION_MAJOR == 6
#include "CameraWidgetQt6.h"
#endif


namespace PokemonAutomation{


std::vector<CameraInfo> get_all_cameras(){
#if QT_VERSION_MAJOR == 5
    return qt5_get_all_cameras();
#elif QT_VERSION_MAJOR == 6
    return qt6_get_all_cameras();
#endif
}
QString get_camera_name(const CameraInfo& info){
#if QT_VERSION_MAJOR == 5
    return qt5_get_camera_name(info);
#elif QT_VERSION_MAJOR == 6
    return qt6_get_camera_name(info);
#endif
}
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
){
    return [&](QWidget& parent){
#if QT_VERSION_MAJOR == 5
        return new Qt5VideoWidget(&parent, logger, info, desired_resolution);
#elif QT_VERSION_MAJOR == 6
        return new Qt6VideoWidget(&parent, logger, info, desired_resolution);
#endif
    };
}


}
