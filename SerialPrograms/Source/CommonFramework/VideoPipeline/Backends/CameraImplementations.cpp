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

//#include <iostream>
//using std::cout;
//using std::endl;

#if 0
#elif QT_VERSION_MAJOR == 6
#include "CameraWidgetQt6.h"
#include "CameraWidgetQt6.5.h"
#include "CameraWidgetQt6_QVideoWidget.h"
#include "CameraWidgetQt6_QML.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
#include "CameraWidgetOpenCV.h"
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
#if defined(__linux__) || defined(__APPLE__)
        m_backends.emplace_back(
            "opencv-v4l2", "Linux: OpenCV V4L2",
            std::make_unique<CameraOpenCV::CameraBackend>()
        );
#endif
#if QT_VERSION_MAJOR == 6
        m_backends.emplace_back(
            "qt6-QVideoSink", "Qt6: QVideoSink",
            std::make_unique<CameraQt6QVideoSink::CameraBackend>()
        );
        m_backends.emplace_back(
            "qt6-qvideowidget", "Qt6: GPU Offloaded (QOpenGLWidget)",
            std::make_unique<CameraQt6QVideoWidget::CameraBackend>()
        );
        m_backends.emplace_back(
            "qt6-qml", "Qt6: GPU Offloaded (QML QQuickWidget)",
            std::make_unique<CameraQt6QML::CameraBackend>()
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
