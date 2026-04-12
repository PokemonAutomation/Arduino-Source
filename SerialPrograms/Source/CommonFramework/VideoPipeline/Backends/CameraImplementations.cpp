/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
#include "CameraImplementations.h"

#ifdef _WIN32
#include "DirectShowCameraList.h"
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

#if 0
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
    IntegerEnumDropdownDatabase m_database;
};



VideoBackendOption::VideoBackendOption()
    : IntegerEnumDropdownOption(
        "<b>Video Pipeline Backend:</b>",
        CameraBackends::instance().m_database,
        LockMode::LOCK_WHILE_RUNNING,
#if 0
#elif QT_VERSION_MAJOR == 6
#if QT_VERSION_MINOR >= 5
        1
#else
        0
#endif
#endif
    )
{}





const CameraBackend& get_camera_backend(){
    size_t index = GlobalSettings::instance().VIDEO_PIPELINE->VIDEO_BACKEND.current_value();
    return *CameraBackends::instance().m_backends[index].backend;
}



std::vector<CameraInfo> get_all_cameras(){
    const CameraBackend& backend = get_camera_backend();
    std::vector<CameraInfo> ret = backend.get_all_cameras();

#ifdef _WIN32
    //  Some capture cards (e.g. AVerMedia GC550) only have DirectShow drivers
    //  and are invisible to Qt6's WMF-based QMediaDevices.  Enumerate DirectShow
    //  devices and append any that Qt didn't find.
    std::vector<std::string> qt_names;
    qt_names.reserve(ret.size());
    for (const CameraInfo& cam : ret){
        qt_names.push_back(backend.get_camera_name(cam));
    }
    for (CameraInfo& dshow_cam : get_directshow_only_cameras(qt_names)){
        ret.push_back(std::move(dshow_cam));
    }
#endif

    return ret;
}
std::string get_camera_name(const CameraInfo& info){
#ifdef _WIN32
    if (is_directshow_device(info)){
        return get_directshow_display_name(info);
    }
#endif
    const CameraBackend& backend = get_camera_backend();
    std::string ret = backend.get_camera_name(info);
    return ret;
}


}
