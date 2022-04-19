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
#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoToolsQt5.h"
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



Qt5VideoWidget::Qt5VideoWidget(
    QWidget* parent,
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : VideoWidget(parent)
    , m_logger(logger)
    , m_last_orientation_attempt(WallClock::min())
//    , m_use_probe_frames(true)
//    , m_flip_vertical(true)
    , m_last_frame_seqnum(0)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
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
//        m_use_probe_frames = false;
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
                WallClock now = current_time();
                SpinLockGuard lg(m_frame_lock);
                if (GlobalSettings::instance().ENABLE_FRAME_SCREENSHOTS){
                    m_last_frame = frame;
                }
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
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
QImage Qt5VideoWidget::direct_snapshot_image(std::unique_lock<std::mutex>& lock){
//    std::unique_lock<std::mutex> lg(m_lock);
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
        lock,
        std::chrono::milliseconds(1000),
        [&]{ return capture.status != CaptureStatus::PENDING; }
    );

    if (capture.status != CaptureStatus::COMPLETED){
        m_logger.log("Capture timed out.");
    }

    QImage image = std::move(capture.image);
    m_pending_captures.erase(iter.first);
    QImage::Format format = m_last_image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    return image;
}
QImage Qt5VideoWidget::direct_snapshot_probe(bool flip_vertical){
//    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    QVideoFrame frame;
    {
        SpinLockGuard lg1(m_frame_lock);
        frame = m_last_frame;
    }

    return frame_to_image(m_logger, frame, flip_vertical);
}
QImage Qt5VideoWidget::snapshot_image(std::unique_lock<std::mutex>& lock, WallClock* timestamp){
//    cout << "snapshot_image()" << endl;
//    std::unique_lock<std::mutex> lg(m_lock);

    auto now = current_time();
    if (timestamp){
        timestamp[0] = now;
    }

    if (m_camera == nullptr){
        return QImage();
    }

    //  Frame is already cached and is not stale.
    uint64_t frame_seqnum;
    {
        SpinLockGuard lg1(m_frame_lock);
        frame_seqnum = m_last_frame_seqnum;
        if (!m_last_image.isNull()){
            if (m_probe){
                //  If we have the probe enabled, we know if a new frame has been pushed.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image_seqnum == frame_seqnum){
//                    cout << "cached 0" << endl;
                    if (timestamp){
                        timestamp[0] = m_last_image_timestamp;
                    }
                    return m_last_image;
                }
            }else{
                //  Otherwise, we have to use time.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image_timestamp + m_frame_period > now){
//                    cout << "cached 1" << endl;
                    if (timestamp){
                        timestamp[0] = m_last_image_timestamp;
                    }
                    return m_last_image;
                }
            }
        }
    }

    WallClock time0 = current_time();

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

//    cout << "snapshot: " << id << endl;
    capture.cv.wait_for(
        lock,
        std::chrono::milliseconds(1000),
        [&]{ return capture.status != CaptureStatus::PENDING; }
    );

    if (capture.status != CaptureStatus::COMPLETED){
        m_logger.log("Capture timed out.");
    }

    m_last_image = std::move(capture.image);
    m_pending_captures.erase(iter.first);
    QImage::Format format = m_last_image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        m_last_image = m_last_image.convertToFormat(QImage::Format_ARGB32);
    }
    m_last_image_timestamp = now;
    m_last_image_seqnum = frame_seqnum;
    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());
    return m_last_image;
}
QImage Qt5VideoWidget::snapshot_probe(WallClock* timestamp){
//    std::lock_guard<std::mutex> lg(m_lock);

    if (m_camera == nullptr){
        if (timestamp){
            timestamp[0] = current_time();
        }
        return QImage();
    }

    //  Frame is already cached and is not stale.
    QVideoFrame frame;
    WallClock frame_timestamp;
    uint64_t frame_seqnum;
    {
//        WallClock time0 = current_time();
        SpinLockGuard lg0(m_frame_lock);
//        WallClock time1 = current_time();
//        cout << "Lock (snapshot): " << std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count() << endl;
        frame_seqnum = m_last_frame_seqnum;
        if (!m_last_image.isNull() && m_last_image_seqnum == frame_seqnum){
//            cout << "cached 0" << endl;
            if (timestamp){
                timestamp[0] = m_last_image_timestamp;
            }
            return m_last_image;
        }
//        WallClock time0 = current_time();
        frame = m_last_frame;
//        WallClock time1 = current_time();
//        cout << "Copy Frame (snapshot): " << std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count() << endl;
        frame_timestamp = m_last_frame_timestamp;
    }

    WallClock time0 = current_time();
    m_last_image = frame_to_image(m_logger, frame, m_flip_vertical);
//    cout << "Convert Frame: " << std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count() << endl;
    m_last_image_timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;
    if (timestamp){
        timestamp[0] = frame_timestamp;
    }
    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());
    return m_last_image;
}

QImage Qt5VideoWidget::snapshot(WallClock* timestamp){
    std::unique_lock<std::mutex> lg(m_lock);

    //  Frame screenshots are disabled.
    if (!GlobalSettings::instance().ENABLE_FRAME_SCREENSHOTS){
        return snapshot_image(lg, timestamp);
    }

    //  QVideoFrame is enabled and ready!
    if (m_probe && m_orientation_known){
        return snapshot_probe(timestamp);
    }

    //  If probing is enabled and we don't know the frame orientation, try to
    //  figure it out. But don't try too often if we fail.
    if (m_probe && !m_orientation_known){
        WallClock now = current_time();
        if (m_last_orientation_attempt + std::chrono::seconds(10) < now){
            m_orientation_known = determine_frame_orientation(lg);
            m_last_orientation_attempt = now;
        }
    }

    if (m_orientation_known){
        return snapshot_probe(timestamp);
    }else{
        return snapshot_image(lg, timestamp);
    }
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





bool Qt5VideoWidget::determine_frame_orientation(std::unique_lock<std::mutex>& lock){
    //  Qt 5.12 is really shitty in that there's no way to figure out the
    //  orientation of a QVideoFrame. So here we'll try to figure it out
    //  the poor man's way. Snapshot using both QCameraImageCapture and
    //  QVideoProbe and compare them.

    //  This function cannot be called on the UI thread.
    //  This function must be called under the lock.

    QImage reference = direct_snapshot_image(lock);
    QImage frame = direct_snapshot_probe(false);
    m_orientation_known = PokemonAutomation::determine_frame_orientation(m_logger, reference, frame, m_flip_vertical);
    return m_orientation_known;
}



























}
}
#endif

