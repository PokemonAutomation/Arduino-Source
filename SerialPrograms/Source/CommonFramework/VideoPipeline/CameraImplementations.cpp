/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "CameraImplementations.h"

#if QT_VERSION_MAJOR == 5
#include "Qt5CameraWidget.h"
#elif QT_VERSION_MAJOR == 6
#include "Qt6CameraWidget.h"
#endif


namespace PokemonAutomation{


#if QT_VERSION_MAJOR == 5

std::vector<CameraInfo> get_all_cameras(){
    return qt5_get_all_cameras();
}
QString get_camera_name(const CameraInfo& info){
    return qt5_get_camera_name(info);
}
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
){
    return [&](QWidget& parent){
        return new Qt5VideoWidget(&parent, logger, info, desired_resolution);
    };
}

#elif QT_VERSION_MAJOR == 6

std::vector<CameraInfo> get_all_cameras(){
    return qt6_get_all_cameras();
}
QString get_camera_name(const CameraInfo& info){
    return qt6_get_camera_name(info);
}
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
){
    return [&](QWidget& parent){
        return new Qt6VideoWidget(&parent, logger, info, desired_resolution);
    };
}

#else
#error "Unknown Qt Version."
#endif






}
