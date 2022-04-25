/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QVBoxLayout>
#include <QCameraInfo>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QVideoProbe>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoToolsQt5.h"
#include "CameraInfo.h"
#include "CameraWidgetQt5v2.h"

namespace PokemonAutomation{
namespace CameraQt5{


CameraHolder::CameraHolder(
    LoggerQt& logger, Qt5VideoWidget2& widget,
    const CameraInfo& info, const QSize& desired_resolution
)
    : m_logger(logger)
    , m_widget(widget)
    , m_last_orientation_attempt(WallClock::min())
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{
    m_camera = new QCamera(QCameraInfo(info.device_name().c_str()), this);

    m_capture = new QCameraImageCapture(m_camera, this);
    m_capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    m_camera->setCaptureMode(QCamera::CaptureStillImage);
    m_camera->start();

    for (const auto& size : m_camera->supportedViewfinderResolutions()){
        m_supported_resolutions.emplace_back(size);
    }

    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    m_max_frame_rate = settings.maximumFrameRate();
    m_frame_period = std::chrono::milliseconds(25);
    if (0 < m_max_frame_rate){
        m_frame_period = std::chrono::milliseconds((uint64_t)(1.0 / m_max_frame_rate * 1000));
    }
    m_current_resolution = settings.resolution();


    //  Check if we can apply the recommended resolution.
    QSize new_resolution = m_current_resolution;
    for (const QSize& size : m_supported_resolutions){
        if (size == desired_resolution){
            new_resolution = desired_resolution;
            break;
        }
    }
    if (m_current_resolution != new_resolution){
        settings.setResolution(new_resolution);
        m_camera->setViewfinderSettings(settings);
        m_current_resolution = new_resolution;
    }

    m_probe = new QVideoProbe(this);
    if (!m_probe->setSource(m_camera)){
        logger.log("Unable to initialize QVideoProbe() capture.", COLOR_RED);
        delete m_probe;
        m_probe = nullptr;
    }

    if (m_probe){
        connect(
            m_probe, &QVideoProbe::videoFrameProbed,
            this, [=](const QVideoFrame& frame){
//                static int c = 0;
//                cout << "asdf: " << c++ << endl;
//                std::terminate();
                WallClock now = current_time();
                SpinLockGuard lg(m_frame_lock);
                if (GlobalSettings::instance().ENABLE_FRAME_SCREENSHOTS){
                    m_last_frame = frame;
                }
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
            },
            Qt::DirectConnection
        );
    }

    connect(
        m_capture, &QCameraImageCapture::imageCaptured,
        this, [&](int id, const QImage& preview){
            std::lock_guard<std::mutex> lg(m_state_lock);
//            cout << "finish = " << id << endl;
            auto iter = m_pending_captures.find(id);
            if (iter == m_pending_captures.end()){
                m_logger.log(
                    "QCameraImageCapture::imageCaptured(): Unable to find capture ID: " + std::to_string(id),
                    COLOR_RED
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
            std::lock_guard<std::mutex> lg(m_state_lock);
//            cout << "error = " << id << endl;
            m_logger.log(
                "QCameraImageCapture::error(): Capture ID: " + errorString,
                COLOR_RED
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
CameraHolder::~CameraHolder(){
    m_camera->stop();
    delete m_capture;
}
void CameraHolder::set_resolution(const QSize& size){
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    if (settings.resolution() == size){
        return;
    }
    settings.setResolution(size);
    m_camera->setViewfinderSettings(settings);
    m_current_resolution = size;
}
QImage CameraHolder::direct_snapshot_image(std::unique_lock<std::mutex>& lock){
//    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QImage();
    }

    m_camera->searchAndLock();
    int id = m_capture->capture();
    m_camera->unlock();

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
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    return image;
}
QImage CameraHolder::direct_snapshot_probe(bool flip_vertical){
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
bool CameraHolder::determine_frame_orientation(std::unique_lock<std::mutex>& lock){
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
VideoSnapshot CameraHolder::snapshot_image(std::unique_lock<std::mutex>& lock){
//    cout << "snapshot_image()" << endl;
//    std::unique_lock<std::mutex> lg(m_lock);

    auto now = current_time();

    if (m_camera == nullptr){
        return VideoSnapshot{QImage(), now};
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
                    return VideoSnapshot{m_last_image, m_last_image_timestamp};
                }
            }else{
                //  Otherwise, we have to use time.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image_timestamp + m_frame_period > now){
//                    cout << "cached 1" << endl;
                    return VideoSnapshot{m_last_image, m_last_image_timestamp};
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
        return VideoSnapshot{QImage(), now};
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
    return VideoSnapshot{m_last_image, m_last_image_timestamp};
}
VideoSnapshot CameraHolder::snapshot_probe(){
//    std::lock_guard<std::mutex> lg(m_lock);

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

    m_last_image = frame_to_image(m_logger, frame, m_flip_vertical);
    m_last_image_timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    return VideoSnapshot{m_last_image, frame_timestamp};
}
VideoSnapshot CameraHolder::snapshot(){
    std::unique_lock<std::mutex> lg(m_state_lock);

    //  Frame screenshots are disabled.
    if (!GlobalSettings::instance().ENABLE_FRAME_SCREENSHOTS){
        return snapshot_image(lg);
    }

    //  QVideoFrame is enabled and ready!
    if (m_probe && m_orientation_known){
        return snapshot_probe();
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
        return snapshot_probe();
    }else{
        return snapshot_image(lg);
    }
}








Qt5VideoWidget2::Qt5VideoWidget2(
    QWidget* parent,
    LoggerQt& logger,
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

    m_holder = std::make_unique<CameraHolder>(logger, *this, info, desired_resolution);
//    m_holder->moveToThread(&m_thread);
//    connect(&m_thread, &QThread::finished, m_holder.get(), &QObject::deleteLater);
//    m_thread.start();

    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_holder->m_camera->setViewfinder(m_camera_view);

    m_holder->moveToThread(&m_thread);
    m_thread.start();
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_qthread(m_thread);

    connect(
        this, &Qt5VideoWidget2::internal_set_resolution,
        m_holder.get(), &CameraHolder::set_resolution
    );
}
Qt5VideoWidget2::~Qt5VideoWidget2(){
    m_holder.reset();
    m_thread.quit();
    m_thread.wait();
}
QSize Qt5VideoWidget2::current_resolution() const{
    if (!m_holder){
        return QSize();
    }
    return m_holder->current_resolution();
}
std::vector<QSize> Qt5VideoWidget2::supported_resolutions() const{
    if (!m_holder){
        return std::vector<QSize>();
    }
    return m_holder->supported_resolutions();
}
void Qt5VideoWidget2::set_resolution(const QSize& size){
    emit internal_set_resolution(size);
}
VideoSnapshot Qt5VideoWidget2::snapshot(){
    if (!m_holder){
        return VideoSnapshot{QImage(), current_time()};
    }
    return m_holder->snapshot();
}
void Qt5VideoWidget2::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
    if (m_holder == nullptr){
        return;
    }
    m_camera_view->setFixedSize(this->size());
}





}
}
#endif
