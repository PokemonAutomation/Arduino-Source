/*  Camera Widget (OpenCV V4L2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if defined(__linux__) || defined(__APPLE__)

#include <QPainter>
#include <QVideoSink>
#include <QResizeEvent>
#include <QMediaDevices>
#include <QVideoFrameFormat>
#include <opencv2/opencv.hpp>
#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#endif
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetOpenCV_QOpenGLWidget.h"

namespace PokemonAutomation{
namespace CameraOpenCV_QOpenGLWidget{

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

#if defined(__linux__)
    int fd = open(info.device_name().c_str(), O_RDWR);
    if (fd >= 0) {
        struct v4l2_fmtdesc fmtdesc = {};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
            VideoFormat fmt = VideoFormat::OTHER;
            bool valid_fmt = false;
            if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG) { fmt = VideoFormat::MJPEG; valid_fmt = true; }
            else if (fmtdesc.pixelformat == V4L2_PIX_FMT_YUYV) { fmt = VideoFormat::YUYV; valid_fmt = true; }
            
            if (valid_fmt) {
                struct v4l2_frmsizeenum frmsize = {};
                frmsize.pixel_format = fmtdesc.pixelformat;
                while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
                    if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                        uint32_t w = frmsize.discrete.width;
                        uint32_t h = frmsize.discrete.height;
                        
                        int ival_index = 0;
                        while (true) {
                            struct v4l2_frmivalenum frmival = {};
                            frmival.index = ival_index;
                            frmival.pixel_format = fmtdesc.pixelformat;
                            frmival.width = w;
                            frmival.height = h;
                            
                            if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) != 0) {
                                break;
                            }
                            
                            if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                                double fps = (double)frmival.discrete.denominator / frmival.discrete.numerator;
                                m_formats[{w, h}][fmt].insert((size_t)std::round(fps));
                            }
                            ival_index++;
                        }
                    }
                    frmsize.index++;
                }
            }
            fmtdesc.index++;
        }
        close(fd);
    }
#endif
    
    // Ensure the desired format is at least present in case probing failed
    if (m_formats[{desired_resolution.width, desired_resolution.height}][desired_format].empty()) {
        m_formats[{desired_resolution.width, desired_resolution.height}][desired_format].insert(desired_fps);
    }

    m_resolution = desired_resolution;
    m_format = desired_format;
    m_fps = desired_fps;

    std::string device_path = info.device_name();
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == device_path){
            m_camera_description = camera.description().toStdString();
            break;
        }
    }
    
#if defined(__linux__)
    int device_index = 0;
    if (device_path.find("/dev/video") == 0) {
        try {
            device_index = std::stoi(device_path.substr(10));
        } catch (...) {}
    }
    m_device_index = device_index;
    m_cap = std::make_unique<cv::VideoCapture>(m_device_index, cv::CAP_V4L2);
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
    int consecutive_failures = 0;
    while (!m_stop.load(std::memory_order_acquire)){
        if (!m_cap->read(bgr) || bgr.empty()){
            consecutive_failures++;
            if (consecutive_failures % 30 == 1) {
                m_logger.log("Failed to read frame from OpenCV. (Failures: " + std::to_string(consecutive_failures) + ")", COLOR_RED);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            if (consecutive_failures > 40) {
                m_logger.log("Too many consecutive read failures. Attempting to restart capture device...", COLOR_RED);
                m_cap->release();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
#if defined(__linux__)
                m_cap->open(m_device_index, cv::CAP_V4L2);
                if (!m_cap->isOpened() && !m_camera_description.empty()) {
                    m_logger.log("Failed to reopen device index " + std::to_string(m_device_index) + ". Searching for it by name...", COLOR_RED);
                    const auto new_cameras = GlobalMediaServices::instance().get_all_cameras();
                    for (const auto& camera : new_cameras) {
                        if (camera.description().toStdString() == m_camera_description) {
                            std::string new_path = camera.id().toStdString();
                            if (new_path.find("/dev/video") == 0) {
                                try {
                                    m_device_index = std::stoi(new_path.substr(10));
                                    m_logger.log("Found device at new index " + std::to_string(m_device_index), COLOR_RED);
                                    m_cap->open(m_device_index, cv::CAP_V4L2);
                                    if (m_cap->isOpened()) break;
                                } catch (...) {}
                            }
                        }
                    }
                }
#endif
                if (m_cap->isOpened()) {
                    if (m_format == VideoFormat::MJPEG){
                        m_cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
                    } else if (m_format == VideoFormat::YUYV){
                        m_cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
                    }
                    m_cap->set(cv::CAP_PROP_FRAME_WIDTH, m_resolution.width);
                    m_cap->set(cv::CAP_PROP_FRAME_HEIGHT, m_resolution.height);
                    if (m_fps > 0) {
                        m_cap->set(cv::CAP_PROP_FPS, m_fps);
                    }
                    consecutive_failures = 0;
                }
            }
            continue;
        }
        consecutive_failures = 0;

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
    : QOpenGLWidget(parent)
    , m_source(source)
    , m_width(80)
    , m_height(45)
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    source.add_source_frame_listener(*this);
}
void CameraVideoDisplay::resizeEvent(QResizeEvent* event){
    QOpenGLWidget::resizeEvent(event);
    m_width.store(event->size().width(), std::memory_order_relaxed);
    m_height.store(event->size().height(), std::memory_order_relaxed);
}

void CameraVideoDisplay::on_frame(std::shared_ptr<const VideoFrame> frame){
    int w = m_width.load(std::memory_order_relaxed);
    int h = m_height.load(std::memory_order_relaxed);
    
    std::shared_ptr<const VideoFrame> scaled_frame = frame;
    
    if (w > 0 && h > 0 && frame && frame->frame.isValid()){
        QSize target_size(w, h);
        if (frame->frame.size() != target_size){
            QImage img = frame->frame.toImage();
            QImage scaled = img.scaled(target_size, Qt::IgnoreAspectRatio, Qt::FastTransformation);
            QVideoFrame out(scaled);
            scaled_frame = std::make_shared<VideoFrame>(frame->timestamp, out);
        }
    }

    QMetaObject::invokeMethod(this, [this, frame = std::move(scaled_frame)]() mutable {
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
