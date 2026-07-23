/*  Camera Widget (OpenCV V4L2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if defined(__linux__) || defined(__APPLE__)

#include <QPainter>
#include <QMediaDevices>
#include <QVideoFrameFormat>
#include <opencv2/opencv.hpp>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetOpenCV.h"

namespace PokemonAutomation{
namespace CameraOpenCV{

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
    m_stop.store(true, std::memory_order_release);
    if (m_thread.joinable()){
        m_thread.join();
    }
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
    , m_stop(false)
    , m_last_frame(logger)
    , m_snapshot_manager(logger, m_last_frame)
{
    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = OpenCV V4L2");

    // Populate fake format set so UI has some options if we don't query it.
    // In a real implementation we would probe, but here we just pass the desired through
    m_formats[{1920, 1080}][VideoFormat::MJPEG] = {60, 30};
    m_formats[{1280, 720}][VideoFormat::MJPEG] = {60, 30};
    m_formats[{1920, 1080}][VideoFormat::YUYV] = {10, 5};
    m_formats[{desired_resolution.width, desired_resolution.height}][desired_format] = {desired_fps};

    m_resolution = desired_resolution;
    m_format = desired_format;
    m_fps = desired_fps;

    std::string device_path = info.device_name();
#if defined(__linux__)
    int device_index = 0;
    if (device_path.find("/dev/video") == 0) {
        try {
            device_index = std::stoi(device_path.substr(10));
        } catch (...) {}
    }
    m_cap = std::make_unique<cv::VideoCapture>(device_index, cv::CAP_V4L2);
#else
    m_cap = std::make_unique<cv::VideoCapture>(device_path);
#endif

    if (!m_cap->isOpened()){
        m_logger.log("Camera failed to open via OpenCV.", COLOR_RED);
        return;
    }

    if (desired_format == VideoFormat::MJPEG){
        m_cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    } else if (desired_format == VideoFormat::YUYV){
        m_cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    }

    m_cap->set(cv::CAP_PROP_FRAME_WIDTH, desired_resolution.width);
    m_cap->set(cv::CAP_PROP_FRAME_HEIGHT, desired_resolution.height);
    if (desired_fps > 0) {
        m_cap->set(cv::CAP_PROP_FPS, desired_fps);
    }

    m_logger.log("Resolution: " + m_resolution.to_string() + ", FPS: " + std::to_string(m_fps));

    m_thread = std::thread(&CameraVideoSource::thread_body, this);
}

void CameraVideoSource::thread_body(){
    cv::Mat bgr, bgra;
    while (!m_stop.load(std::memory_order_acquire)){
        if (!m_cap->read(bgr)){
            continue; // Could sleep or break on disconnect
        }

        WallClock now = current_time();

        cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);

        QVideoFrameFormat format(QSize(bgra.cols, bgra.rows), QVideoFrameFormat::Format_BGRA8888);
        QVideoFrame frame(format);

        if (frame.map(QVideoFrame::WriteOnly)){
            memcpy(frame.bits(0), bgra.data, bgra.total() * bgra.elemSize());
            frame.unmap();
            
            if (!m_last_frame.push_frame(frame, now)){
                continue;
            }
            report_source_frame(std::make_shared<VideoFrame>(now, frame));
        }
    }
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
    m_last_frame = std::move(frame);
    queue_on_main_thread([this]{
        this->update();
    });
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
