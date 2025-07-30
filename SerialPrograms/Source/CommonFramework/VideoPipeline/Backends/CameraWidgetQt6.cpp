/*  Camera Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <chrono>
#include <iostream>
#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVideoSink>
//#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Time.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetQt6.h"

//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{



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
    if (!m_capture){
        return;
    }
    try{
        m_logger.log("Stopping Camera...");
    }catch (...){}

//    m_camera->stop();
    m_capture.reset();
    m_video_sink.reset();
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
    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt6QVideoSink");

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
    m_video_sink.reset(new QVideoSink());
    m_capture.reset(new QMediaCaptureSession());
    m_capture->setCamera(&m_camera->camera());
    m_capture->setVideoSink(m_video_sink.get());

#if 0
    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });
#endif
    connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
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





CameraVideoDisplay::~CameraVideoDisplay(){
    m_source.remove_source_frame_listener(*this);
}
CameraVideoDisplay::CameraVideoDisplay(QWidget* parent, CameraVideoSource& source)
    : QWidget(parent)
    , m_source(source)
{
    source.add_source_frame_listener(*this);
}
void CameraVideoDisplay::on_frame(std::shared_ptr<const VideoFrame> frame){
    m_last_frame = frame;
    this->update();
}
void CameraVideoDisplay::paintEvent(QPaintEvent* event){
    QWidget::paintEvent(event);

    if (!m_last_frame){
        return;
    }

    QVideoFrame frame = m_last_frame->frame;
    if (!frame.isValid()){
        return;
    }

    QRect rect(0, 0, this->width(), this->height());
    QVideoFrame::PaintOptions options;
    QPainter painter(this);

    frame.paint(&painter, rect, options);
    m_source.report_rendered_frame(current_time());
}



















}
}
#endif
