/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CameraImplementations.h"

#if QT_VERSION_MAJOR == 5
#include "VideoToolsQt5.h"
#include "CameraWidgetQt5.h"
#include "CameraWidgetQt5v2.h"
#elif QT_VERSION_MAJOR == 6
#include "CameraWidgetQt6.h"
#endif


namespace PokemonAutomation{


struct CameraBackends{
    static const CameraBackends& instance(){
        static CameraBackends backends;
        return backends;
    }

    CameraBackends(){
#if QT_VERSION_MAJOR == 5
        m_backends.emplace_back("Qt5: QCameraViewfinder",                   new CameraQt5QCameraViewfinder::CameraBackend());
        m_backends.emplace_back("Qt5: QCameraViewfinder (separate thread)", new CameraQt5QCameraViewfinderSeparateThread::CameraBackend());
#endif
#if QT_VERSION_MAJOR == 6
        m_backends.emplace_back("Qt6: QVideoSink",                          new CameraQt6QVideoSink::CameraBackend());
#endif

        for (const auto& item : m_backends){
            m_labels.emplace_back(item.first);
        }
    }

    std::vector<std::pair<QString, std::unique_ptr<CameraBackend>>> m_backends;
    std::vector<QString> m_labels;
};



VideoBackendOption::VideoBackendOption()
    : EnumDropdownOption(
        "<b>Video Pipeline Backend:</b>",
        CameraBackends::instance().m_labels,
#if QT_VERSION_MAJOR == 5
        1
#elif QT_VERSION_MAJOR == 6
        0
#endif
    )
{}







std::vector<CameraInfo> get_all_cameras(){
    const CameraBackend& backend = *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
    return backend.get_all_cameras();
}
QString get_camera_name(const CameraInfo& info){
    const CameraBackend& backend = *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
    return backend.get_camera_name(info);
}
std::function<VideoWidget*(QWidget& parent)> make_video_factory(
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
){
    return [&](QWidget& parent){
        const CameraBackend& backend = *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
        return backend.make_video_widget(parent, logger, info, desired_resolution);
    };
}


}
