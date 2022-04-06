/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QCameraInfo>
#include <QVBoxLayout>
#include "Common/Compiler.h"
#include "CameraWidgetQt5.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt5{


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


#if 0
QImage QVideoFrame_to_QImage(QVideoFrame& frame){
    //  "frame" must already be mapped.

    QImage ret;

    QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
    do{
        if (format != QImage::Format_Invalid){
            ret = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), format).copy();
            break;
        }
        if (frame.pixelFormat() == QVideoFrame::Format_Jpeg){
            ret.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
            break;
        }
    }while (false);
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        ret = ret.convertToFormat(QImage::Format_ARGB32);
    }
    return ret;
}
#endif




Qt5VideoWidget::Qt5VideoWidget(
    QWidget* parent,
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : VideoWidget(parent)
    , m_logger(logger)
    , m_last_snapshot(std::chrono::system_clock::time_point::min())
    , m_seqnum_frame(0)
{
    if (!info){
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    m_camera = new QCamera(QCameraInfo(info.device_name().c_str()), this);

#if 1
    m_probe = new QVideoProbe(this);
    if (!m_probe->setSource(m_camera)){
        logger.log("Unable to initialize QVideoProbe() capture.", COLOR_RED);
        delete m_probe;
        m_probe = nullptr;
    }
#endif
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
    m_max_frame_rate = settings.maximumFrameRate();
    m_frame_period = std::chrono::milliseconds(25);
    if (0 < m_max_frame_rate){
        m_frame_period = std::chrono::milliseconds((uint64_t)(1.0 / m_max_frame_rate * 1000));
    }
    m_resolution = settings.resolution();

//    cout << "min = " << settings.minimumFrameRate() << endl;
//    cout << "max = " << settings.maximumFrameRate() << endl;


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

    if (m_probe){
        connect(
            m_probe, &QVideoProbe::videoFrameProbed,
            this, [=](const QVideoFrame& frame){
//                SpinLockGuard lg(m_frame_lock);
//                m_last_frame = frame;
                m_seqnum_frame++;
//                cout << "asdf" << endl;
            }
        );
    }
    {
        connect(
            m_capture, &QCameraImageCapture::imageCaptured,
            this, [&](int id, const QImage& preview){
                std::lock_guard<std::mutex> lg(m_lock);
//                cout << "finish = " << id << endl;
                auto iter = m_pending_captures.find(id);
                if (iter == m_pending_captures.end()){
                    m_logger.log(
                        "QCameraImageCapture::imageCaptured(): Unable to find capture ID: " + std::to_string(id),
                        COLOR_RED
                    );
//                    cout << "QCameraImageCapture::imageCaptured(): Unable to find capture ID: " << id << endl;
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
//                cout << "error = " << id << endl;
                m_logger.log(
                    "QCameraImageCapture::error(): Capture ID: " + errorString,
                    COLOR_RED
                );
//                cout << "QCameraImageCapture::error(): " << errorString.toUtf8().data() << endl;
                auto iter = m_pending_captures.find(id);
                if (iter == m_pending_captures.end()){
                    return;
                }
                iter->second.status = CaptureStatus::COMPLETED;
                iter->second.cv.notify_all();
            }
        );
    }
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
#if 0
QImage Qt5VideoWidget::snapshot_probe(){
//    cout << "snapshot_probe()" << endl;
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    QVideoFrame frame;
//    SpinLockGuard lg0(m_capture_lock);
    {
        SpinLockGuard lg1(m_frame_lock);
        if (m_seqnum_image == m_seqnum_frame){
            return m_last_image;
        }
        frame = std::move(m_last_frame);
        m_seqnum_image = m_seqnum_frame;
    }
    if (!frame.isValid()){
        return QImage();
    }
    if (!frame.map(QAbstractVideoBuffer::ReadOnly)){
        m_logger.log("Unable to map QVideoFrame.", COLOR_RED);
    }
    QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
//    cout << "field = " << (int)frame.fieldType() << endl;
//    cout << "format = " << (int)format << endl;
    QImage image(
        frame.bits(),
        frame.width(),
        frame.height(),
        frame.bytesPerLine(),
        format
    );
//    image.save("test.png");
#ifdef _WIN32
    m_last_image = image.convertToFormat(QImage::Format_RGB32).mirrored(false, true);
#else
    m_last_image = image.convertToFormat(QImage::Format_RGB32).copy();
#endif
    return m_last_image;
}
#endif
QImage Qt5VideoWidget::snapshot_image(){
//    cout << "snapshot_image()" << endl;
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    //  Frame is already cached and is not stale.
    auto timestamp = std::chrono::system_clock::now();
#if 1
    {
        SpinLockGuard lg1(m_frame_lock);
        if (!m_last_image.isNull()){
            if (m_probe){
                //  If we have the probe enabled, we know if a new frame has been pushed.
//                cout << timestamp - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_seqnum_image == m_seqnum_frame.load(std::memory_order_acquire)){
//                    cout << "cached 0" << endl;
                    return m_last_image;
                }
            }else{
                //  Otherwise, we have to use time.
//                cout << timestamp - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_snapshot.load(std::memory_order_acquire) + m_frame_period > timestamp){
//                    cout << "cached 1" << endl;
                    return m_last_image;
                }
            }
        }
    }
#endif

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
    QImage::Format format = ret.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        ret = ret.convertToFormat(QImage::Format_ARGB32);
    }
    m_last_image = ret;
    m_last_snapshot.store(timestamp, std::memory_order_release);
    m_seqnum_image = m_seqnum_frame.load(std::memory_order_acquire);
    return ret;
}
QImage Qt5VideoWidget::snapshot(){
    return snapshot_image();
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
}
#endif

