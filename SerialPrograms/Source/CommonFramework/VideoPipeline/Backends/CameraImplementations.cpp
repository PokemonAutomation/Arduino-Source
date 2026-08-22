/*  Camera Implementations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
#include "CameraImplementations.h"

//#include <iostream>
//using std::cout;
//using std::endl;

#include "CameraWidgetQt6.h"
#include "CameraWidgetQt6.5.h"
#include "CameraWidgetQt6_QQuickWidget.h"
#include "CameraWidgetQt6_QQuickView.h"


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
        m_backends.emplace_back(
            "qt6-QVideoSink", "Qt6: QVideoSink",
            std::make_unique<CameraQt6QVideoSink::CameraBackend>()
        );
        m_backends.emplace_back(
            "qt6.5-QGraphicsScene", "Qt6.5: QGraphicsScene",
            std::make_unique<CameraQt65QMediaCaptureSession::CameraBackend>()
        );
        if (STATIC_GLOBALS.DEVELOPER_MODE){
            m_backends.emplace_back(
                "qt6-QQuickWidget", "Qt6: QQuickWidget",
                std::make_unique<CameraQt6_QQuickWidget::CameraBackend>()
            );
            m_backends.emplace_back(
                "qt6-QQuickView", "Qt6: QQuickView",
                std::make_unique<CameraQt6_QQuickView::CameraBackend>()
            );
        }

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
        1
    )
{}





const CameraBackend& get_camera_backend(){
    size_t index = GlobalSettings::instance().VIDEO_PIPELINE->VIDEO_BACKEND.current_value();
    const CameraBackends& backends = CameraBackends::instance();
    if (index >= backends.m_backends.size()){
        index = 1;
    }
    return *backends.m_backends[index].backend;
}



std::vector<CameraInfo> get_all_cameras(){
    const CameraBackend& backend = get_camera_backend();
//    global_logger_tagged().log("Start loading camera list...");
//    WallClock start = current_time();
    std::vector<CameraInfo> ret = backend.get_all_cameras();
//    WallClock end = current_time();
//    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
//    global_logger_tagged().log("Done loading camera list... " + tostr_fixed(seconds, 3) + " seconds");
    return ret;
}
std::string get_camera_name(const CameraInfo& info){
    const CameraBackend& backend = get_camera_backend();
//    global_logger_tagged().log("Start reading camera name...");
//    WallClock start = current_time();
    std::string ret = backend.get_camera_name(info);
//    WallClock end = current_time();
//    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
//    global_logger_tagged().log("Done reading camera name... " + tostr_fixed(seconds, 3) + " seconds");
    return ret;
}


}
