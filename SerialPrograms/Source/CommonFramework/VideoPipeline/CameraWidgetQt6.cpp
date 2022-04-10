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
#include "CameraWidgetQt6.h"

#include <chrono>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt6{


std::vector<CameraInfo> qt6_get_all_cameras(){
    std::vector<CameraInfo> ret;
    const auto cameras = QMediaDevices::videoInputs();
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}

QString qt6_get_camera_name(const CameraInfo& info){
    const auto cameras = QMediaDevices::videoInputs();
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            return camera.description();
        }
    }
    std::cout << "Error: no such camera for CameraInfo: " << info.device_name() << std::endl;
    return "";
}


Qt6VideoWidget::Qt6VideoWidget(
    QWidget* parent,
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : VideoWidget(parent)
    , m_logger(logger)
    , m_seqnum_frame(0)
{
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
        std::cout << "Cannot build Qt6VideoWidget: wrong camera device name: " << info.device_name() << std::endl;
        return;
    }

    m_camera = new QCamera(m_info, this);
    m_captureSession.setCamera(m_camera);

    connect(m_camera, &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });

    m_videoSink = new QVideoSink(this);
    m_captureSession.setVideoOutput(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, [&](const QVideoFrame& frame){
        {
            auto timestamp = current_time();
            SpinLockGuard lg(m_frame_lock);
            m_videoFrame = frame;
            m_videoTimestamp = timestamp;
            uint64_t seqnum = m_seqnum_frame.load(std::memory_order_acquire);
            m_seqnum_frame.store(seqnum + 1, std::memory_order_release);
//            m_seqnum_frame++;
        }
        this->update();
    });

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
        m_resolutions.emplace_back(format.resolution());
    }

    this->setMinimumSize(80, 45);

    //  Check if we can apply the recommended resolution.
    for (const auto& format : m_formats){
        if (format.resolution() == desired_resolution){
            m_camera->setCameraFormat(format);
            break;
        }
    }

    m_camera->start();
}
Qt6VideoWidget::~Qt6VideoWidget(){}

QSize Qt6VideoWidget::resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QSize();
    }
    return m_camera->cameraFormat().resolution();
}

std::vector<QSize> Qt6VideoWidget::resolutions() const{
    return m_resolutions;
}

void Qt6VideoWidget::set_resolution(const QSize& size){
    std::lock_guard<std::mutex> lg(m_lock);
    {
        const auto format = m_camera->cameraFormat();
        if (format.resolution() == size){
            return;
        }
    }
    bool formatSet = false;
    for(const auto& format : m_formats){
        if (format.resolution() == size){
            m_camera->setCameraFormat(format);
            formatSet = true;
            break;
        }
    }
    if (formatSet == false){
        std::cout << "Wrong resolution: " << size.width() << "x" << size.height() << ", camera does not support it" << std::endl;
    }
}

QImage Qt6VideoWidget::snapshot(WallClock* timestamp){
    //  Prevent multiple concurrent screenshots from entering here.
    std::lock_guard<std::mutex> lg(m_image_lock);

    //  Image is already cached and not stale. Return it.
    uint64_t seqnum = m_seqnum_frame.load(std::memory_order_acquire);
    if (m_seqnum_image == seqnum && !m_cached_frame.isNull()){
        if (timestamp){
            timestamp[0] = m_cached_timestamp;
        }
        return m_cached_frame;
    }

    //  Need to update image. Grab the current frame.
    QVideoFrame frame;
    WallClock tick;
    {
        SpinLockGuard lg1(m_frame_lock);
        frame = m_videoFrame;   // Fast due to ref-count.
        tick = m_videoTimestamp;
    }

    //  Now make the image.
    if (!frame.isValid()){
        if (timestamp){
            timestamp[0] = current_time();
        }
        return QImage();
    }

    // auto time1 = current_time();
    //  Convert image and cache it.
    QImage image = frame.toImage();
    // auto time2 = current_time();
    // std::chrono::duration<double> elapsed_seconds = time2 - time1;
    // std::cout << "snapshot image conversion time " << elapsed_seconds.count() << "s" << std::endl;
    // std::cout << "QVideoFrame pixel format " << int(frame.pixelFormat()) << std::endl;
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    m_seqnum_image = seqnum;
    m_cached_frame = std::move(frame);
    if (timestamp){
        timestamp[0] = m_cached_timestamp;
    }
    return m_cached_frame;
}


void Qt6VideoWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
    if (m_camera == nullptr){
        return;
    }

#if 0
    cout << "Widget = " << this->width() << " x " << this->height() << endl;

    QSize size = m_camera_view->size();
    cout << "Camera = " << size.width() << " x " << size.height() << endl;

    QSize hint = m_camera_view->sizeHint();
    cout << "Hint = " << hint.width() << " x " << hint.height() << endl;
#endif

    this->setFixedSize(this->size());
}

void Qt6VideoWidget::paintEvent(QPaintEvent* event){
    // std::cout << "paintEvent start" << std::endl;
    QWidget::paintEvent(event);

    //  Lock should not be needed since it's only updated on this UI thread.
//    std::lock_guard<std::mutex> lg(m_lock);

    if (m_videoFrame.isValid()){
        QRect rect(0,0, this->width(), this->height());
        QVideoFrame::PaintOptions options;
        QPainter painter(this);
        m_videoFrame.paint(&painter, rect, options);
    }
    // std::cout << "paintEvent end" << std::endl;
}



}
}
#endif
