/*  Camera Widget (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR >= 5

#include <chrono>
#include <iostream>
#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QImageCapture>
//#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Time.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/VideoPipeline/CameraOption.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
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
std::unique_ptr<PokemonAutomation::CameraSession> CameraBackend::make_camera(Logger& logger, Resolution default_resolution) const{
    return std::make_unique<CameraSession>(logger, default_resolution);
}







void CameraSession::add_state_listener(StateListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_listeners.insert(&listener);
}
void CameraSession::remove_state_listener(StateListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_listeners.erase(&listener);
}
void CameraSession::add_frame_listener(VideoFrameListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_listeners.insert(&listener);
}
void CameraSession::remove_frame_listener(VideoFrameListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_frame_listeners.erase(&listener);
}

CameraSession::~CameraSession(){
    global_watchdog().remove(*this);
    shutdown();
}
CameraSession::CameraSession(Logger& logger, Resolution default_resolution)
    : m_logger(logger)
    , m_default_resolution(default_resolution)
    , m_resolution(default_resolution)
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
    , m_last_frame_seqnum(0)
    , m_last_image_timestamp(WallClock::min())
//    , m_history(GlobalSettings::instance().HISTORY_SECONDS * 1000000)
{
    uint8_t watchdog_timeout = GlobalSettings::instance().VIDEO_PIPELINE->AUTO_RESET_SECONDS;
    if (watchdog_timeout != 0){
        global_watchdog().add(*this, std::chrono::seconds(watchdog_timeout));
    }
}

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
    m_logger.log("Resetting the video...", COLOR_GREEN);
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
        if (!m_capture_session){
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
    ReadSpinLock lg(m_frame_lock);
    return {m_last_frame, m_last_frame_seqnum};
}
void CameraSession::report_rendered_frame(WallClock timestamp){
    {
        WriteSpinLock lg(m_frame_lock);
        m_fps_tracker_display.push_event(timestamp);
    }
    global_watchdog().delay(*this);
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
        ReadSpinLock lg0(m_frame_lock);
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
    ReadSpinLock lg(m_frame_lock);
    return m_fps_tracker_source.events_per_second();
}
double CameraSession::fps_display(){
    ReadSpinLock lg(m_frame_lock);
    return m_fps_tracker_display.events_per_second();
}

void CameraSession::connect_video_sink(QVideoSink* sink){
#if 1
    connect(
        sink, &QVideoSink::videoFrameChanged,
        this, [&](const QVideoFrame& frame){
            WallClock now = current_time();
            {
                WriteSpinLock lg(m_frame_lock);

                //  Skip duplicate frames.
                if (frame.startTime() <= m_last_frame.startTime()){
                    return;
                }

                m_last_frame = frame;
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
//                m_history.push_frame(frame);
                m_fps_tracker_source.push_event(now);
            }
//            cout << now_to_filestring() << endl;
            std::lock_guard<std::mutex> lg(m_lock);

            if (!m_frame_listeners.empty()){
                std::shared_ptr<VideoFrame> frame_ptr(new VideoFrame(now, frame));
                for (VideoFrameListener* listener : m_frame_listeners){
                    listener->on_frame(frame_ptr);
                }
            }

        },
        Qt::DirectConnection
    );
#endif
}
void CameraSession::clear_video_output(){
    m_video_sink.reset(new QVideoSink());
    connect_video_sink(m_video_sink.get());
}
void CameraSession::set_video_output(QVideoWidget& widget){
    if (m_capture_session == nullptr){
        return;
    }
    if (m_capture_session->videoSink() == widget.videoSink()){
        return;
    }
    m_capture_session->setVideoOutput(&widget);
    connect_video_sink(widget.videoSink());
}
void CameraSession::set_video_output(QGraphicsVideoItem& item){
    if (m_capture_session == nullptr){
        return;
    }
    if (m_capture_session->videoSink() == item.videoSink()){
        return;
    }
    m_capture_session->setVideoOutput(&item);
    connect_video_sink(item.videoSink());
}

void CameraSession::shutdown(){
    if (!m_capture_session){
        return;
    }
    m_logger.log("Stopping Camera...");

    for (StateListener* listener : m_state_listeners){
        listener->pre_shutdown();
    }
    m_camera->stop();
    m_capture_session.reset();
//    m_video_sink.reset();
    m_camera.reset();
    m_resolution_map.clear();
    m_formats.clear();

    {
        WriteSpinLock lg(m_frame_lock);

        m_last_frame = QVideoFrame();
        m_last_frame_timestamp = current_time();
        m_last_frame_seqnum++;

        m_last_image = QImage();
        m_last_image_timestamp = m_last_frame_timestamp;
        m_last_image_seqnum = m_last_frame_seqnum;
    }

    for (StateListener* listener : m_state_listeners){
        listener->post_shutdown();
    }
}
void CameraSession::startup(){
    if (!m_device){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt65QMediaCaptureSession");

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

    QSize size = desired_format->resolution();
    m_resolution = Resolution(size.width(), size.height());

    m_camera.reset(new QCamera(*device));
    m_camera->setCameraFormat(*desired_format);
//    m_video_sink.reset(new QVideoSink());
    m_capture_session.reset(new QMediaCaptureSession());
    m_capture_session->setCamera(m_camera.get());
//    m_capture_session->setVideoSink(m_video_sink.get());

//    QImageCapture* capture = new QImageCapture(this);
//    capture-

    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() == QCamera::NoError){
            return;
        }
        m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
#if 0
        QMetaObject::invokeMethod(this, [this]{
            reset();
        }, Qt::QueuedConnection);
#endif
    });
    clear_video_output();

    m_camera->start();

//    cout << "frame rate = " << m_camera->cameraFormat().minFrameRate() << endl;

    for (StateListener* listener : m_state_listeners){
        listener->post_new_source(m_device, m_resolution);
    }
}

void CameraSession::on_watchdog_timeout(){
    m_logger.log("CameraSession::on_watchdog_timeout()", COLOR_RED);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (!m_device){
            return;
        }
    }

    uint8_t watchdog_timeout = GlobalSettings::instance().VIDEO_PIPELINE->AUTO_RESET_SECONDS;
    m_logger.log("No video detected for " + std::to_string(watchdog_timeout) + " seconds...", COLOR_RED);

    if (watchdog_timeout == 0){
        return;
    }
    reset();
}





PokemonAutomation::VideoWidget* CameraSession::make_QtWidget(QWidget* parent){
    return new VideoDisplayWidget(parent, *this);
}




VideoDisplayWidget::VideoDisplayWidget(QWidget* parent, CameraSession& camera)
    : PokemonAutomation::VideoWidget(parent)
    , m_session(camera)
#ifdef PA_USE_QVideoWidget
    , m_widget(new QVideoWidget(this))
#else
    , m_view(new StaticQGraphicsView(this))
#endif
{
    this->setMinimumSize(80, 45);

#ifdef PA_USE_QVideoWidget
    m_widget->setFixedSize(this->size());
    camera.set_video_output(*m_widget);
#else
    m_view->setFixedSize(this->size());
    m_view->setScene(&m_scene);
    m_video.setSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_scene.addItem(&m_video);
    camera.set_video_output(m_video);

    connect(
        &m_scene, &QGraphicsScene::changed,
        this, [&](const QList<QRectF>&){
            m_session.report_rendered_frame(current_time());
        }
    );
#endif

    m_session.add_state_listener(*this);
}
VideoDisplayWidget::~VideoDisplayWidget(){
    m_session.remove_state_listener(*this);
}

//void VideoDisplayWidget::new_frame_available(){
//    this->update();
//}

void VideoDisplayWidget::pre_shutdown(){
    m_session.clear_video_output();
}
void VideoDisplayWidget::post_new_source(const CameraInfo& device, Resolution resolution){
#ifdef PA_USE_QVideoWidget
    m_session.set_video_output(*m_widget);
#else
    m_session.set_video_output(m_video);
#endif
}

void VideoDisplayWidget::resizeEvent(QResizeEvent* event){
#ifdef PA_USE_QVideoWidget
    m_widget->setFixedSize(this->size());
#else
    m_view->setFixedSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_video.setSize(this->size());
#endif
}


#if 0
void VideoDisplayWidget::paintEvent(QPaintEvent* event){
    // std::cout << "paintEvent start" << std::endl;
    QWidget::paintEvent(event);

//    cout << m_camera->isActive() << endl;
//    cout << m_widget->videoSink() << endl;
//    cout << m_widget->width() << " x " << m_widget->height() << endl;

    QPainter painter(this);
    painter.setPen(QColor((uint32_t)COLOR_RED));
    painter.drawRect(0, 0, 500, 500);

#if 0
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

    frame.first.paint(&painter, rect, options);
    // std::cout << "paintEvent end" << std::endl;

    if (m_last_seqnum != frame.second){
        m_last_seqnum = frame.second;
        m_session.report_rendered_frame(current_time());
    }
#endif
}
#endif




















}
}
#endif
