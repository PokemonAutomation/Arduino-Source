/*  Camera Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "CommonFramework/VideoPipeline/CameraOption.h"
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
std::unique_ptr<PokemonAutomation::CameraSession> CameraBackend::make_camera(Logger& logger, Resolution default_resolution) const{
    return std::make_unique<CameraSession>(logger, default_resolution);
}







void CameraSession::add_state_listener(StateListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_listeners.insert(&listener);
}
void CameraSession::remove_state_listener(StateListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_listeners.erase(&listener);
}
void CameraSession::add_listener(FrameReadyListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_ready_listeners.insert(&listener);
}
void CameraSession::remove_listener(FrameReadyListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_ready_listeners.erase(&listener);
}
void CameraSession::add_frame_listener(VideoFrameListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_listeners.insert(&listener);
}
void CameraSession::remove_frame_listener(VideoFrameListener& listener){
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_listeners.erase(&listener);
}

CameraSession::~CameraSession(){
    shutdown();
}
CameraSession::CameraSession(Logger& logger, Resolution default_resolution)
    : m_logger(logger)
    , m_default_resolution(default_resolution)
    , m_resolution(default_resolution)
    , m_last_frame_seqnum(0)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{}

void CameraSession::get(CameraOption& option){
    std::lock_guard<std::mutex> lg(m_lock);
    option.info = m_device;
    option.current_resolution = m_resolution;
}
void CameraSession::set(const CameraOption& option){
    QMetaObject::invokeMethod(this, [this, option]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        m_device = option.info;
        m_resolution = option.current_resolution;
        startup();
    });
}
void CameraSession::reset(){
    QMetaObject::invokeMethod(this, [this]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        startup();
    });
}
void CameraSession::set_source(CameraInfo device){
//    cout << "CameraSession::set_source()" << endl;
    QMetaObject::invokeMethod(this, [this, device]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        m_device = std::move(device);
        startup();
    });
}
void CameraSession::set_resolution(Resolution resolution){
    QMetaObject::invokeMethod(this, [this, resolution]{
        std::lock_guard<std::mutex> lg(m_lock);
        if (!m_capture){
            m_resolution = resolution;
            return;
        }
        m_logger.log("Setting resolution to: " + resolution.to_string());
        auto iter = m_resolution_map.find(resolution);
        if (iter == m_resolution_map.end()){
            m_logger.log("Resolution not supported.", COLOR_RED);
            return;
        }
        for (StateListener* listener : m_state_listeners){
            listener->pre_resolution_change(resolution);
        }
        m_resolution = resolution;
        m_camera->stop();
        m_camera->setCameraFormat(*iter->second);
        m_camera->start();
        for (StateListener* listener : m_state_listeners){
            listener->post_resolution_change(resolution);
        }
    });
}
CameraInfo CameraSession::current_device() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_device;
}
Resolution CameraSession::current_resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_resolution;
}
std::vector<Resolution> CameraSession::supported_resolutions() const{
    std::lock_guard<std::mutex> lg(m_lock);
    std::vector<Resolution> ret;
    for (const auto& item : m_resolution_map){
        ret.emplace_back(item.first);
    }
    return ret;
}
std::pair<QVideoFrame, uint64_t> CameraSession::latest_frame(){
    SpinLockGuard lg(m_frame_lock);
    return {m_last_frame, m_last_frame_seqnum};
}
void CameraSession::report_rendered_frame(WallClock timestamp){
    SpinLockGuard lg(m_frame_lock);
    m_fps_tracker_display.push_event(timestamp);
}

VideoSnapshot CameraSession::snapshot(){
    //  Prevent multiple concurrent screenshots from entering here.
    std::lock_guard<std::mutex> lg(m_lock);

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

    m_last_image = std::move(image);
    m_last_image_timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    return VideoSnapshot(m_last_image, m_last_image_timestamp);
}
double CameraSession::fps_source(){
    SpinLockGuard lg(m_frame_lock);
    return m_fps_tracker_source.events_per_second();
}
double CameraSession::fps_display(){
    SpinLockGuard lg(m_frame_lock);
    return m_fps_tracker_display.events_per_second();
}


void CameraSession::shutdown(){
    if (!m_capture){
        return;
    }
    m_logger.log("Stopping Camera...");

    m_camera->stop();
    for (StateListener* listener : m_state_listeners){
        listener->pre_shutdown();
    }
    m_capture.reset();
    m_video_sink.reset();
    m_camera.reset();
    m_resolution_map.clear();
    m_formats.clear();

    SpinLockGuard lg(m_frame_lock);

    m_last_frame = QVideoFrame();
    m_last_frame_timestamp = current_time();
    m_last_frame_seqnum++;

    m_last_image = QImage();
    m_last_image_timestamp = m_last_frame_timestamp;
    m_last_image_seqnum = m_last_frame_seqnum;

}
void CameraSession::startup(){
    if (!m_device){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt6QVideoSink");

    auto cameras = QMediaDevices::videoInputs();
    const QCameraDevice* device = nullptr;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == m_device.device_name()){
            device = &camera;
            break;
        }
    }
    if (device == nullptr){
        m_logger.log("Camera not found: " + m_device.device_name(), COLOR_RED);
        return;
    }

    m_formats = device->videoFormats();
    if (m_formats.empty()){
        m_logger.log("No usable resolutions: " + device->description().toStdString(), COLOR_RED);
        return;
    }

    m_resolution_map.clear();
    for (const QCameraFormat& format : m_formats){
        QSize resolution = format.resolution();
        m_resolution_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(resolution.width(), resolution.height()),
            std::forward_as_tuple(&format)
        );
    }

    const QCameraFormat* default_format = nullptr;
    const QCameraFormat* desired_format = nullptr;
    m_resolutions.clear();
    for (const auto& item : m_resolution_map){
        m_resolutions.emplace_back(item.first);
        if (item.first == m_default_resolution){
            default_format = item.second;
        }
        if (item.first == m_resolution){
            desired_format = item.second;
        }
    }
    if (desired_format == nullptr){
        desired_format = default_format;
    }
    if (desired_format == nullptr){
        desired_format = m_resolution_map.rbegin()->second;
    }
//    cout << "CameraSession::m_resolutions = " << m_resolutions.size() << endl;

    QSize size = desired_format->resolution();
    m_resolution = Resolution(size.width(), size.height());

    m_camera.reset(new QCamera(*device));
    m_camera->setCameraFormat(*desired_format);
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
        this, [&](const QVideoFrame& frame){
            WallClock now = current_time();
            {
                SpinLockGuard lg(m_frame_lock);
//                WallClock start = current_time();
                m_last_frame = frame;
//                cout << std::chrono::duration_cast<std::chrono::microseconds>(current_time() - start).count() << endl;
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
                m_fps_tracker_source.push_event(now);
            }
//            cout << now_to_filestring() << endl;
            std::lock_guard<std::mutex> lg(m_lock);
            for (FrameReadyListener* listener : m_frame_ready_listeners){
                listener->new_frame_available();
            }
            std::shared_ptr<VideoFrame> frame_ptr(new VideoFrame(now, frame));
            for (VideoFrameListener* listener : m_frame_listeners){
                listener->on_frame(frame_ptr);
            }
        }
    );

    m_camera->start();

    for (StateListener* listener : m_state_listeners){
        listener->post_new_source(m_device, m_resolution);
    }
}

PokemonAutomation::VideoWidget* CameraSession::make_QtWidget(QWidget* parent){
    return new VideoWidget(parent, *this);
}




VideoWidget::VideoWidget(QWidget* parent, CameraSession& camera)
    : PokemonAutomation::VideoWidget(parent)
    , m_session(camera)
{
    this->setMinimumSize(80, 45);
    m_session.add_listener(*this);
}
VideoWidget::~VideoWidget(){
    m_session.remove_listener(*this);
}

void VideoWidget::new_frame_available(){
    this->update();
}
void VideoWidget::paintEvent(QPaintEvent* event){
    // std::cout << "paintEvent start" << std::endl;
    QWidget::paintEvent(event);

    //  Lock should not be needed since it's only updated on this UI thread.
//    std::lock_guard<std::mutex> lg(m_lock);

    std::pair<QVideoFrame, uint64_t> frame = m_session.latest_frame();
    if (!frame.first.isValid()){
        return;
    }

//    cout << "frame: " << this->width() << " x " << this->height() << endl;

    QRect rect(0, 0, this->width(), this->height());
    QVideoFrame::PaintOptions options;
    QPainter painter(this);

//    WallClock start = current_time();
    frame.first.paint(&painter, rect, options);
    // std::cout << "paintEvent end" << std::endl;
//    cout << "paint = " << std::chrono::duration_cast<std::chrono::microseconds>(current_time() - start).count() << endl;

    if (m_last_seqnum != frame.second){
        m_last_seqnum = frame.second;
        m_session.report_rendered_frame(current_time());
    }
}




















}
}
#endif
