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
//#include "CameraWidgetQt5v2.h"
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
//        if (PreloadSettings::instance().DEVELOPER_MODE){
//            m_backends.emplace_back("Qt5: QCameraViewfinder (separate thread)", new CameraQt5QCameraViewfinderSeparateThread::CameraBackend());
//        }
#endif
#if QT_VERSION_MAJOR == 6
        m_backends.emplace_back("Qt6: QVideoSink",                          new CameraQt6QVideoSink::CameraBackend());
#endif

        for (const auto& item : m_backends){
            m_cases.emplace_back(item.first);
        }
    }

    std::vector<std::pair<std::string, std::unique_ptr<CameraBackend>>> m_backends;
    std::vector<EnumDropdownOption::Option> m_cases;
};



VideoBackendOption::VideoBackendOption()
    : EnumDropdownOption(
        "<b>Video Pipeline Backend:</b>",
        CameraBackends::instance().m_cases,
#if QT_VERSION_MAJOR == 5
        0
#elif QT_VERSION_MAJOR == 6
        0
#endif
    )
{}







std::vector<CameraInfo> get_all_cameras(){
    const CameraBackend& backend = *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
    return backend.get_all_cameras();
}
std::string get_camera_name(const CameraInfo& info){
    const CameraBackend& backend = *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
    return backend.get_camera_name(info);
}

const CameraBackend& get_camera_backend(){
    return *CameraBackends::instance().m_backends[GlobalSettings::instance().VIDEO_BACKEND].second;
}



}
