/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVideoSink>
#include "Common/Cpp/Exceptions.h"
#include "CameraWidgetQt6.h"

#include <chrono>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{



std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
    std::vector<CameraInfo> ret;
    const auto cameras = QMediaDevices::videoInputs();
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
    const auto cameras = QMediaDevices::videoInputs();
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            return camera.description().toStdString();
        }
    }
    std::cout << "Error: no such camera for CameraInfo: " << info.device_name() << std::endl;
    return "";
}
std::unique_ptr<PokemonAutomation::Camera> CameraBackend::make_camera(
    Logger& logger,
    const CameraInfo& info,
    const Resolution& desired_resolution
) const{
    return std::make_unique<Camera>(logger, info, desired_resolution);
}
VideoWidget* CameraBackend::make_video_widget(QWidget* parent, PokemonAutomation::Camera& camera) const{
    Camera* casted = dynamic_cast<Camera*>(&camera);
    if (casted == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching camera session/widget types.");
    }
    return new VideoWidget2(parent, *casted);
}



Camera::Camera(
    Logger& logger,
    const CameraInfo& info, const Resolution& desired_resolution
)
    : m_logger(logger)
    , m_last_frame_seqnum(0)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{
    logger.log("Constructing Camera: Backend = CameraQt6QVideoSink");
    if (!info){
        return;
    }

    const auto cameras = QMediaDevices::videoInputs();
    bool foundInfo = false;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            m_info = camera;
            foundInfo = true;
            break;
        }
    }
    if (foundInfo == false){
        std::cout << "Cannot build VideoWidget: wrong camera device name: " << info.device_name() << std::endl;
        return;
    }

    m_camera = new QCamera(m_info, this);

    connect(m_camera, &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });

    m_videoSink = new QVideoSink(this);

    connect(
        m_videoSink, &QVideoSink::videoFrameChanged,
        this, [&](const QVideoFrame& frame){
            {
                WallClock now = current_time();
                SpinLockGuard lg(m_frame_lock);
                m_last_frame = frame;
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
            }
            std::lock_guard<std::mutex> lg(m_lock);
            for (Listener* listener : m_listeners){
                listener->new_frame_available();
            }
        }
    );

    const QList<QCameraFormat> formats = m_info.videoFormats();
    // Filter out additional formats that have the same resolutions:
    for (const auto& format : formats){
        bool foundExistingResolution = false;
        for (size_t i = 0; i < m_formats.size(); i++){
            if (m_formats[i].resolution() == format.resolution()){
                foundExistingResolution = true;
                break;
            }
        }
        if (foundExistingResolution == false){
            m_formats.push_back(format);
        }
    }

    for (const auto& format : m_formats){
        QSize resolution = format.resolution();
        m_resolutions.emplace_back(Resolution(resolution.width(), resolution.height()));
    }

    //  Check if we can apply the recommended resolution.
    for (const auto& format : m_formats){
        QSize resolution = format.resolution();
        if (Resolution(resolution.width(), resolution.height()) == desired_resolution){
            m_camera->setCameraFormat(format);
            break;
        }
    }

    m_camera->start();
}
Camera::~Camera(){}

void Camera::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void Camera::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}

Resolution Camera::current_resolution() const{
    m_sanitizer.check_usage();
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return Resolution();
    }
    QSize size = m_camera->cameraFormat().resolution();
    if (size.isValid()){
        return Resolution(size.width(), size.height());
    }else{
        return Resolution();
    }
}
std::vector<Resolution> Camera::supported_resolutions() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_resolutions;
}
void Camera::set_resolution(const Resolution& size){
//    cout << "set_resolution(): " << size.width() << " x " << size.height() << endl;
    std::lock_guard<std::mutex> lg(m_lock);
    {
        const QCameraFormat format = m_camera->cameraFormat();
        QSize resolution = format.resolution();
        if (Resolution(resolution.width(), resolution.height()) == size){
            return;
        }
    }
    bool formatSet = false;
    for(const auto& format : m_formats){
        QSize resolution = format.resolution();
        if (Resolution(resolution.width(), resolution.height()) == size){
            m_camera->stop();
            m_camera->setCameraFormat(format);
            m_camera->start();
            formatSet = true;
            break;
        }
    }
    if (formatSet == false){
        m_logger.log(
            "Camera doesn't support: " + std::to_string(size.width) + " x " + std::to_string(size.height),
            COLOR_RED
        );
    }
}

QVideoFrame Camera::latest_frame(){
    SpinLockGuard lg(m_frame_lock);
    return m_last_frame;
}
VideoSnapshot Camera::snapshot(){
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






VideoWidget2::VideoWidget2(QWidget* parent, Camera& camera)
    : PokemonAutomation::VideoWidget(parent)
    , m_camera(camera)
{
    if (m_camera.m_camera == nullptr){
        return;
    }

    m_captureSession.setCamera(m_camera.m_camera);
    m_captureSession.setVideoOutput(m_camera.m_videoSink);

    this->setMinimumSize(80, 45);

    m_camera.add_listener(*this);
}
VideoWidget2::~VideoWidget2(){
    m_camera.remove_listener(*this);
}

void VideoWidget2::new_frame_available(){
    this->update();
}
void VideoWidget2::paintEvent(QPaintEvent* event){
    // std::cout << "paintEvent start" << std::endl;
    QWidget::paintEvent(event);

    //  Lock should not be needed since it's only updated on this UI thread.
//    std::lock_guard<std::mutex> lg(m_lock);

    QVideoFrame frame = m_camera.latest_frame();
    if (frame.isValid()){
//        cout << "frame: " << m_last_frame.width() << " x " << m_last_frame.height() << endl;
        QRect rect(0,0, this->width(), this->height());
        QVideoFrame::PaintOptions options;
        QPainter painter(this);
        frame.paint(&painter, rect, options);
    }
    // std::cout << "paintEvent end" << std::endl;
}


















}
}
#endif
