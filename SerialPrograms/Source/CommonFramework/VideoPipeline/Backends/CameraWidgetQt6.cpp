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

    m_camera->stop();
    m_capture.reset();
    m_video_sink.reset();
    m_camera.reset();
}
CameraVideoSource::CameraVideoSource(
    Logger& logger,
    const CameraInfo& info,
    Resolution desired_resolution
)
    : VideoSource(true)
    , m_logger(logger)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
    , m_last_frame_seqnum(0)
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

    m_camera.reset(new QCamera(*device));
    m_camera->setCameraFormat(*format);
    m_video_sink.reset(new QVideoSink());
    m_capture.reset(new QMediaCaptureSession());
    m_capture->setCamera(m_camera.get());
    m_capture->setVideoSink(m_video_sink.get());

    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });
    connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
        m_camera.get(), [&](const QVideoFrame& frame){
            //  This will be on the main thread. So we waste as little time as
            //  possible. Shallow-copy the frame, update the listeners, and
            //  return immediately to unblock the main thread.

            WallClock now = current_time();
            {
                WriteSpinLock lg(m_frame_lock);
                m_last_frame = frame;
                m_last_frame_timestamp = now;
                uint64_t seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
                seqnum++;
                m_last_frame_seqnum.store(seqnum, std::memory_order_relaxed);
            }
            report_source_frame(std::make_shared<VideoFrame>(now, frame));
        }
    );

    m_camera->start();
}

VideoSnapshot CameraVideoSource::snapshot(){
    //  Prevent multiple concurrent screenshots from entering here.
    std::lock_guard<std::mutex> lg(m_snapshot_lock);

    if (m_camera == nullptr){
        return VideoSnapshot();
    }

    //  Frame is already cached and is not stale.
    QVideoFrame frame;
    WallClock frame_timestamp;
    uint64_t frame_seqnum;
    {
        SpinLockGuard lg0(m_frame_lock);
        frame_seqnum = m_last_frame_seqnum;
        if (!m_last_image.isNull() && m_last_image_seqnum == frame_seqnum){
            return VideoSnapshot(m_last_image, m_last_image_timestamp);
        }
        frame = m_last_frame;
        frame_timestamp = m_last_frame_timestamp;
    }

    if (!frame.isValid()){
        global_logger_tagged().log("QVideoFrame is null.", COLOR_RED);
        return VideoSnapshot();
    }

    WallClock time0 = current_time();

    QImage image = frame.toImage();
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    //  No lock needed here since this the only place that touches it.
    m_last_image = std::move(image);
    m_last_image_timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    return VideoSnapshot(m_last_image, m_last_image_timestamp);
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
