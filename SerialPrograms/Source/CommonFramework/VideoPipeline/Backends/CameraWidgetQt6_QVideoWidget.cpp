/*  Camera Widget (Qt6 QVideoWidget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QCamera>
#include <QVideoSink>
#include <QPainter>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "CameraWidgetQt6.h"
#include "CameraWidgetQt6_QVideoWidget.h"
#include "MediaServicesQt6.h"
#include "VideoFrameQt.h"

namespace PokemonAutomation{
namespace CameraQt6QVideoWidget{

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
    global_logger_tagged().log("Error: No such camera for CameraInfo: " + info.device_name(), COLOR_RED);
    return "";
}

std::unique_ptr<VideoSource> CameraBackend::make_video_source(
    Logger& logger,
    const CameraInfo& info,
    Resolution resolution,
    VideoFormat format,
    FramesPerSecond fps
) const{
    return std::make_unique<CameraVideoSource>(logger, info, resolution, format, fps);
}


CameraVideoSource::~CameraVideoSource(){
    if (!m_capture){
        return;
    }
    try{
        m_logger.log("Stopping Camera...");
    }catch (...){}

    run_on_main_thread_and_wait([&]{
        m_metaobject.reset();
        m_capture.reset();
        m_video_sink.reset();
        m_camera.reset();
    });
}
CameraVideoSource::CameraVideoSource(
    Logger& logger,
    const CameraInfo& info,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
)
    : VideoSource(logger, true)
    , m_logger(logger)
    , m_last_frame(logger)
    , m_snapshot_manager(logger, m_last_frame)
{
    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt6QVideoWidget");

    run_on_main_thread_and_wait([&]{
        init(info, desired_resolution, desired_format, desired_fps);
    });
}
void CameraVideoSource::init(
    const CameraInfo& info,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
){
    m_metaobject.reset(new QObject());

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

    QCameraFormat format = CameraQt6QVideoSink::build_format_set(
        m_logger,
        m_formats,
        *device,
        desired_resolution,
        desired_format,
        desired_fps
    );
    if (format.isNull()){
        return;
    }

    CameraQt6QVideoSink::get_format(format, m_resolution, m_format, m_fps);
    m_logger.log(
        "Resolution: " + m_resolution.to_string() +
        ", Format: " + VideoFormat_database().find(m_format)->display +
        ", FPS: " + std::to_string(m_fps)
    );

    m_camera.reset(new QCameraThread(m_logger, *device, format));
    m_video_sink.reset(new QVideoSink());
    m_capture.reset(new QMediaCaptureSession());
    m_capture->setCamera(&m_camera->camera());
    m_capture->setVideoSink(m_video_sink.get());

    m_metaobject->connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
        &m_camera->camera(), [&](const QVideoFrame& frame){
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
    : QOpenGLWidget(parent)
    , m_source(source)
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    source.add_source_frame_listener(*this);
}

void CameraVideoDisplay::on_frame(std::shared_ptr<const VideoFrame> frame){
    QMetaObject::invokeMethod(this, [this, frame = std::move(frame)]() mutable {
        m_last_frame = std::move(frame);
        this->update();
    }, Qt::QueuedConnection);
}

void CameraVideoDisplay::paintEvent(QPaintEvent* event){
    QOpenGLWidget::paintEvent(event);

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
