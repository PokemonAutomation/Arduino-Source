/*  Camera Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCamera>
#include <QPainter>
#include <QVideoSink>
#include "Common/Qt/Redispatch.h"
#include "VideoFrameQt.h"
#include "QFormatAggregator.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetQt6.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{






std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
    std::vector<CameraInfo> ret;
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
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

//    cout << "Stopping Camera..." << endl;

    run_on_main_thread_and_wait([&]{
        m_metaobject.reset();
//        m_camera->stop();
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
    m_logger.log("Starting Camera: Backend = CameraQt6QVideoSink");

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

    QCameraFormat format = build_format_set(
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

    get_format(format, m_resolution, m_format, m_fps);
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

#if 0
    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });
#endif
    m_metaobject->connect(
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
