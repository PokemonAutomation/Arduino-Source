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
    , m_last_frame_seqnum(0)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
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
            this->update();
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

QSize Qt6VideoWidget::current_resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QSize();
    }
    return m_camera->cameraFormat().resolution();
}

std::vector<QSize> Qt6VideoWidget::supported_resolutions() const{
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

VideoSnapshot Qt6VideoWidget::snapshot(){
    //  Prevent multiple concurrent screenshots from entering here.
    std::lock_guard<std::mutex> lg(m_lock);

    if (m_camera == nullptr){
        return VideoSnapshot{QImage(), current_time()};
    }

    //  Frame is already cached and is not stale.
    QVideoFrame frame;
    WallClock frame_timestamp;
    uint64_t frame_seqnum;
    {
        SpinLockGuard lg0(m_frame_lock);
        frame_seqnum = m_last_frame_seqnum;
        if (!m_last_image.isNull() && m_last_image_seqnum == frame_seqnum){
            return VideoSnapshot{m_last_image, m_last_image_timestamp};
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

    return VideoSnapshot{m_last_image, m_last_image_timestamp};
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

    if (m_last_frame.isValid()){
        QRect rect(0,0, this->width(), this->height());
        QVideoFrame::PaintOptions options;
        QPainter painter(this);
        m_last_frame.paint(&painter, rect, options);
    }
    // std::cout << "paintEvent end" << std::endl;
}



}
}
#endif
