/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QPainter>
#include "Qt6CameraWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


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
        if (camera.id().toStdString() == info.device_name()) {
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
{
    if (!info){
        return;
    }

    const auto cameras = QMediaDevices::videoInputs();
    bool foundInfo = false;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()) {
            m_info = camera;
            foundInfo = true;
            break;
        }
    }
    if (foundInfo == false) {
        std::cout << "Cannot build Qt6VideoWidget: wrong camera device name: " << info.device_name() << std::endl;
        return;
    }

    m_camera = new QCamera(m_info, this);
    m_captureSession.setCamera(m_camera);

    connect(m_camera, &QCamera::errorOccurred, this, [&]() {
        if (m_camera->error() != QCamera::NoError) {
            m_logger.log("QCamera error: " + m_camera->errorString());
        }
    });

    m_capture = new QImageCapture(this);
    m_captureSession.setImageCapture(m_capture);
    m_videoSink = new QVideoSink(this);
    m_captureSession.setVideoOutput(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, [&](const QVideoFrame& frame) {
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_videoFrame = frame;
        }
        // std::cout << "Video frame changed, " << frame.width() << "x" << frame.height() << std::endl;
        this->repaint();
        // std::cout << "QtVideoWidget repaint() end in slot" << std::endl;
    });

    const auto formats = m_info.videoFormats();
    for (const auto& format : formats){
        m_resolutions.emplace_back(format.resolution());
    }

    m_resolution = m_camera->cameraFormat().resolution();

    this->setMinimumSize(80, 45);

    //  Check if we can apply the recommended resolution.
    QSize new_resolution = m_resolution;
    for (const QSize& size : m_resolutions){
        if (size == desired_resolution){
            new_resolution = desired_resolution;
            break;
        }
    }
    if (m_resolution != new_resolution){
        const auto formats = m_info.videoFormats();
        for(const auto& format : formats) {
            if (format.resolution() == new_resolution) {
                m_camera->setCameraFormat(format);
                break;
            }
        }
        m_resolution = new_resolution;
    }

    m_camera->start();
}
Qt6VideoWidget::~Qt6VideoWidget(){

}

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
    const auto format = m_camera->cameraFormat();
    if (format.resolution() == size) {
        return;
    }
    const auto formats = m_info.videoFormats();
    bool formatSet = false;
    for(const auto& format : formats) {
        if (format.resolution() == size) {
            m_camera->setCameraFormat(format);
            formatSet = true;
            break;
        }
    }
    if (formatSet == false) {
        std::cout << "Wrong resolution: " << size.width() << "x" << size.height() << ", camera does not support it" << std::endl;
    }
    m_resolution = size;
}

QImage Qt6VideoWidget::snapshot(){
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    QImage ret = m_videoFrame.toImage();

    return ret;
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
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_videoFrame.isValid()) {
        QRect rect(0,0, m_videoFrame.width(), m_videoFrame.height());
        QVideoFrame::PaintOptions options;
        QPainter painter(this);
        m_videoFrame.paint(&painter, rect, options);
    }
    // std::cout << "paintEvent end" << std::endl;
}



}
#endif
