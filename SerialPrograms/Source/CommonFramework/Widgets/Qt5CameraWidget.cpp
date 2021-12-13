/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QCameraInfo>
#include <QVBoxLayout>
#include "Qt5CameraWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


std::vector<CameraInfo> qt5_get_all_cameras(){
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    std::vector<CameraInfo> ret;
    for (const QCameraInfo& info : cameras){
        ret.emplace_back(info.deviceName().toStdString());
    }
    return ret;
}

QString qt5_get_camera_name(const CameraInfo& info){
    QCameraInfo qinfo(info.device_name().c_str());
    return qinfo.description();
}

Qt5VideoWidget::Qt5VideoWidget(
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

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    m_camera = new QCamera(QCameraInfo(info.device_name().c_str()), this);

    m_capture = new QCameraImageCapture(m_camera, this);
    m_capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    m_camera->setCaptureMode(QCamera::CaptureStillImage);

    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_camera->setViewfinder(m_camera_view);
    m_camera->start();

    for (const auto& size : m_camera->supportedViewfinderResolutions()){
        m_resolutions.emplace_back(size);
    }

    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    m_resolution = settings.resolution();

    //  Check if we can apply the recommended resolution.
    QSize new_resolution = m_resolution;
    for (const QSize& size : m_resolutions){
        if (size == desired_resolution){
            new_resolution = desired_resolution;
            break;
        }
    }
    if (m_resolution != new_resolution){
        settings.setResolution(new_resolution);
        m_camera->setViewfinderSettings(settings);
        m_resolution = new_resolution;
    }

    connect(
        m_capture, &QCameraImageCapture::imageCaptured,
        this, [&](int id, const QImage& preview){
            std::lock_guard<std::mutex> lg(m_lock);
//            cout << "finish = " << id << endl;
            auto iter = m_pending_captures.find(id);
            if (iter == m_pending_captures.end()){
                m_logger.log(
                    "QCameraImageCapture::imageCaptured(): Unable to find capture ID: " + std::to_string(id),
                    "red"
                );
//                cout << "QCameraImageCapture::imageCaptured(): Unable to find capture ID: " << id << endl;
                return;
            }
            iter->second.status = CaptureStatus::COMPLETED;
            iter->second.image = preview;
            iter->second.cv.notify_all();
        }
    );
    connect(
        m_capture, static_cast<void(QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString&)>(&QCameraImageCapture::error),
        this, [&](int id, QCameraImageCapture::Error error, const QString& errorString){
            std::lock_guard<std::mutex> lg(m_lock);
//            cout << "error = " << id << endl;
            m_logger.log(
                "QCameraImageCapture::error(): Capture ID: " + errorString,
                "red"
            );
//            cout << "QCameraImageCapture::error(): " << errorString.toUtf8().data() << endl;
            auto iter = m_pending_captures.find(id);
            if (iter == m_pending_captures.end()){
                return;
            }
            iter->second.status = CaptureStatus::COMPLETED;
            iter->second.cv.notify_all();
        }
    );
}
Qt5VideoWidget::~Qt5VideoWidget(){
    for (auto& item : m_pending_captures){
        item.second.status = CaptureStatus::COMPLETED;
        item.second.cv.notify_all();
    }
}
QSize Qt5VideoWidget::resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QSize();
    }
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    return settings.resolution();
}
std::vector<QSize> Qt5VideoWidget::resolutions() const{
    return m_resolutions;
}
void Qt5VideoWidget::set_resolution(const QSize& size){
    std::lock_guard<std::mutex> lg(m_lock);
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    if (settings.resolution() == size){
        return;
    }
    settings.setResolution(size);
    m_camera->setViewfinderSettings(settings);
    m_resolution = size;
}
QImage Qt5VideoWidget::snapshot(){
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    m_camera->searchAndLock();
    int id = m_capture->capture();
    m_camera->unlock();

//    cout << "start = " << id << endl;

    auto iter = m_pending_captures.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple()
    );
    if (!iter.second){
        return QImage();
    }
    PendingCapture& capture = iter.first->second;

    capture.cv.wait_for(
        lg,
        std::chrono::milliseconds(1000),
        [&]{ return capture.status != CaptureStatus::PENDING; }
    );

    if (capture.status != CaptureStatus::COMPLETED){
        m_logger.log("Capture timed out.");
    }

    QImage ret = std::move(capture.image);
    m_pending_captures.erase(iter.first);
    return ret;
}

void Qt5VideoWidget::resizeEvent(QResizeEvent* event){
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

    m_camera_view->setFixedSize(this->size());
}



}
#endif

