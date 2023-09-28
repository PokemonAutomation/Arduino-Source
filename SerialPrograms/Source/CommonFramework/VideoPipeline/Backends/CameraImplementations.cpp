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
#if QT_VERSION_MINOR >= 5
#include "CameraWidgetQt6.5.h"
#endif
#endif


namespace PokemonAutomation{


struct CameraEntry{
    std::string slug;
    std::string display;
    std::unique_ptr<CameraBackend> backend;

    CameraEntry(
        std::string p_slug,
        std::string p_display,
        std::unique_ptr<CameraBackend> p_backend
    )
        : slug(std::move(p_slug))
        , display(std::move(p_display))
        , backend(std::move(p_backend))
    {}
};


struct CameraBackends{
    static const CameraBackends& instance(){
        static CameraBackends backends;
        return backends;
    }

    CameraBackends(){
#if QT_VERSION_MAJOR == 5
        m_backends.emplace_back(
            "qt5-QCameraViewfinder", "Qt5: QCameraViewfinder",
            std::make_unique<CameraQt5QCameraViewfinder::CameraBackend>()
        );
#endif
#if QT_VERSION_MAJOR == 6
        m_backends.emplace_back(
            "qt6-QVideoSink", "Qt6: QVideoSink",
            std::make_unique<CameraQt6QVideoSink::CameraBackend>()
        );
#endif
#if QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR >= 5
        m_backends.emplace_back(
            "qt6.5-QGraphicsScene", "Qt6.5: QGraphicsScene",
            std::make_unique<CameraQt65QMediaCaptureSession::CameraBackend>()
        );
#endif

        size_t items = 0;
        for (const auto& item : m_backends){
            m_database.add(items, item.slug, item.display, true);
            items++;
        }
    }

    std::vector<CameraEntry> m_backends;
    IntegerEnumDatabase m_database;
};



VideoBackendOption::VideoBackendOption()
    : IntegerEnumDropdownOption(
        "<b>Video Pipeline Backend:</b>",
        CameraBackends::instance().m_database,
        LockWhileRunning::LOCKED,
#if QT_VERSION_MAJOR == 5
        0
#elif QT_VERSION_MAJOR == 6
#if QT_VERSION_MINOR >= 5
        1
#else
        0
#endif
#endif
    )
{}







std::vector<CameraInfo> get_all_cameras(){
    size_t index = GlobalSettings::instance().VIDEO_BACKEND.current_value();
    const CameraBackend& backend = *CameraBackends::instance().m_backends[index].backend;
    return backend.get_all_cameras();
}
std::string get_camera_name(const CameraInfo& info){
    size_t index = GlobalSettings::instance().VIDEO_BACKEND.current_value();
    const CameraBackend& backend = *CameraBackends::instance().m_backends[index].backend;
    return backend.get_camera_name(info);
}
const CameraBackend& get_camera_backend(){
    size_t index = GlobalSettings::instance().VIDEO_BACKEND.current_value();
    return *CameraBackends::instance().m_backends[index].backend;
}



}
