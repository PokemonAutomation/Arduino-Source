/*  Camera Widget (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR >= 5

#include <chrono>
#include <iostream>
//#include <QThread>
#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QImageCapture>
//#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Time.h"
//#include "Common/Cpp/PrettyPrint.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetQt6.5.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt65QMediaCaptureSession{






std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    std::vector<CameraInfo> ret;
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            return camera.description().toStdString();
        }
    }
    std::cout << "Error: no such camera for CameraInfo: " << info.device_name() << std::endl;
    return "";
}
std::unique_ptr<VideoSource> CameraBackend::make_video_source(
    Logger& logger,
    const CameraInfo& info,
    Resolution resolution
) const{
    return std::make_unique<CameraVideoSource>(logger, info, resolution);
}







CameraVideoSource::~CameraVideoSource(){
    if (!m_capture_session){
        return;
    }
    try{
        m_logger.log("Stopping Camera...");
    }catch (...){}

//    m_camera->stop();
    m_capture_session.reset();
    m_camera.reset();
}
CameraVideoSource::CameraVideoSource(
    Logger& logger,
    const CameraInfo& info,
    Resolution desired_resolution
)
    : VideoSource(logger, true)
    , m_logger(logger)
    , m_last_frame(logger)
    , m_snapshot_manager(logger, m_last_frame)
{
//    cout << "desired_resolution = " << desired_resolution.width << " x " << desired_resolution.height << endl;

    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt65QMediaCaptureSession");

    auto cameras = QMediaDevices::videoInputs();
    const QCameraDevice* device = nullptr;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            device = &camera;
            break;
        }
    }
    if (device == nullptr){
        m_logger.log("Camera not found: " + info.device_name(), COLOR_RED);
        return;
    }
    m_logger.log("Camera: " + device->description().toStdString());

    QList<QCameraFormat> formats = device->videoFormats();
    if (formats.empty()){
        m_logger.log("No usable resolutions: " + device->description().toStdString(), COLOR_RED);
        return;
    }

    std::map<Resolution, const QCameraFormat*> resolution_map;
    for (const QCameraFormat& format : formats){
        QSize resolution = format.resolution();
        resolution_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(resolution.width(), resolution.height()),
            std::forward_as_tuple(&format)
        );
    }

    const QCameraFormat* format = nullptr;
    m_resolutions.clear();
    for (const auto& res : resolution_map){
        m_resolutions.emplace_back(res.first);
        if (res.first == desired_resolution){
            format = res.second;
        }
    }
    if (format == nullptr){
        format = resolution_map.rbegin()->second;
    }

    QSize size = format->resolution();
    m_resolution = Resolution(size.width(), size.height());
    m_logger.log("Resolution: " + m_resolution.to_string());

    m_camera.reset(new QCameraThread(m_logger, *device, *format));

    m_capture_session.reset(new QMediaCaptureSession());
    m_capture_session->setCamera(&m_camera->camera());

#if 0
    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() == QCamera::NoError){
            return;
        }
        m_logger.log("QCamera error: " + m_camera->errorString().toStdString(), COLOR_RED);
    });

    m_camera->start();
#endif

}


void CameraVideoSource::set_video_output(QGraphicsVideoItem& item){
    if (m_capture_session == nullptr){
        return;
    }
    if (m_capture_session->videoSink() == item.videoSink()){
        return;
    }
    m_capture_session->setVideoOutput(&item);

    connect(
        item.videoSink(), &QVideoSink::videoFrameChanged,
        &m_camera->camera(), [&](const QVideoFrame& frame){
            //  This runs on the QCamera's thread. So it is off the critical path.

            WallClock now = current_time();
            if (!m_last_frame.push_frame(frame, now)){
                return;
            }
            report_source_frame(std::make_shared<VideoFrame>(now, frame));
        }
    );
}






QWidget* CameraVideoSource::make_display_QtWidget(QWidget* parent){
    return new CameraVideoDisplay(parent, *this);
}








CameraVideoDisplay::CameraVideoDisplay(QWidget* parent, CameraVideoSource& source)
    : QWidget(parent)
    , m_source(source)
    , m_view(new StaticQGraphicsView(this))
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    m_view->setFixedSize(this->size());
    m_view->setScene(&m_scene);
    m_video.setSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_scene.addItem(&m_video);
    source.set_video_output(m_video);

    connect(
        &m_scene, &QGraphicsScene::changed,
        this, [&](const QList<QRectF>&){
            auto scope_check = m_sanitizer.check_scope();
            m_source.report_rendered_frame(current_time());
        }
    );
}
void CameraVideoDisplay::resizeEvent(QResizeEvent* event){
    auto scope_check = m_sanitizer.check_scope();
    m_view->setFixedSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_video.setSize(this->size());
}















}
}
#endif
