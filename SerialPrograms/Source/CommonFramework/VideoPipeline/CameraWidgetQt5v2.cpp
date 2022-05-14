/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QApplication>
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
namespace CameraQt5QCameraViewfinderSeparateThread{



std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
    return qt5_get_all_cameras();
}
QString CameraBackend::get_camera_name(const CameraInfo& info) const{
    return qt5_get_camera_name(info);
}
PokemonAutomation::VideoWidget* CameraBackend::make_video_widget(
    QWidget& parent,
    LoggerQt& logger,
    const CameraInfo& info,
    const QSize& desired_resolution
) const{
    return new VideoWidget(&parent, logger, info, desired_resolution);
}




CameraHolder::CameraHolder(
    LoggerQt& logger, VideoWidget& widget,
    const CameraInfo& info, const QSize& desired_resolution
)
    : m_logger(logger)
    , m_widget(widget)
    , m_camera(new QCamera(QCameraInfo(info.device_name().c_str()), this))
    , m_screenshotter(logger, *m_camera)
    , m_last_orientation_attempt(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{
//    m_capture = new QCameraImageCapture(m_camera, this);
//    m_capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
//    m_camera->setCaptureMode(QCamera::CaptureStillImage);
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

    m_probe = new QVideoProbe(m_camera);
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
//                frame_to_image(m_logger, frame, true);
            },
            Qt::DirectConnection
        );
    }
    connect(
        this, &CameraHolder::stop,
        this, [=]{
            this->moveToThread(QApplication::instance()->thread());
            m_camera->stop();

            std::lock_guard<std::mutex> lg(m_state_lock);
            m_stopped = true;
            m_cv.notify_all();
        }
    );
}
CameraHolder::~CameraHolder(){
    //  Redispatch to the thread that owns the class.
//    m_camera->stop();
//    delete m_capture;

    emit this->stop();
    std::unique_lock<std::mutex> lg(m_state_lock);
    m_cv.wait(lg, [=]{ return m_stopped; });
}
void CameraHolder::set_resolution(const QSize& size){
    std::unique_lock<std::mutex> lg(m_state_lock);
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    if (settings.resolution() == size){
        return;
    }
    settings.setResolution(size);
    m_camera->setViewfinderSettings(settings);
    m_current_resolution = size;
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
bool CameraHolder::determine_frame_orientation(){
    //  Qt 5.12 is really shitty in that there's no way to figure out the
    //  orientation of a QVideoFrame. So here we'll try to figure it out
    //  the poor man's way. Snapshot using both QCameraImageCapture and
    //  QVideoProbe and compare them.

    //  This function cannot be called on the UI thread.
    //  This function must be called under the lock.

    QImage reference = m_screenshotter.snapshot().frame;
    QImage frame = direct_snapshot_probe(false);
    m_orientation_known = PokemonAutomation::determine_frame_orientation(m_logger, reference, frame, m_flip_vertical);
    return m_orientation_known;
}
VideoSnapshot CameraHolder::snapshot_image(){
//    cout << "snapshot_image()" << endl;
//    std::unique_lock<std::mutex> lg(m_lock);

    auto now = current_time();

    //  Frame is already cached and is not stale.
    uint64_t frame_seqnum;
    {
        SpinLockGuard lg1(m_frame_lock);
        frame_seqnum = m_last_frame_seqnum;
        if (!m_last_image.frame.isNull()){
            if (m_probe){
                //  If we have the probe enabled, we know if a new frame has been pushed.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image_seqnum == frame_seqnum){
//                    cout << "cached 0" << endl;
                    return m_last_image;
                }
            }else{
                //  Otherwise, we have to use time.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image.timestamp + m_frame_period > now){
//                    cout << "cached 1" << endl;
                    return m_last_image;
                }
            }
        }
    }

    m_last_image = m_screenshotter.snapshot();
    m_last_image_seqnum = frame_seqnum;

    return m_last_image;
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
        if (!m_last_image.frame.isNull() && m_last_image_seqnum == frame_seqnum){
            return m_last_image;
        }
        frame = m_last_frame;
        frame_timestamp = m_last_frame_timestamp;
    }

    WallClock time0 = current_time();

    m_last_image.frame = frame_to_image(m_logger, frame, m_flip_vertical);
    m_last_image.timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    return m_last_image;
}
VideoSnapshot CameraHolder::snapshot(){
    std::unique_lock<std::mutex> lg(m_state_lock);

    //  Frame screenshots are disabled.
    if (!GlobalSettings::instance().ENABLE_FRAME_SCREENSHOTS){
        return snapshot_image();
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
            m_orientation_known = determine_frame_orientation();
            m_last_orientation_attempt = now;
        }
    }

    if (m_orientation_known){
        return snapshot_probe();
    }else{
        return snapshot_image();
    }
}








VideoWidget::VideoWidget(
    QWidget* parent,
    LoggerQt& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : PokemonAutomation::VideoWidget(parent)
    , m_logger(logger)
{
    logger.log("Constructing VideoWidget: Backend = CameraQt5QCameraViewfinderSeparateThread");
    if (!info){
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    m_holder = std::make_unique<CameraHolder>(logger, *this, info, desired_resolution);

    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_holder->m_camera->setViewfinder(m_camera_view);
//    m_holder->m_camera->setViewfinder((QVideoWidget*)nullptr);

    m_holder->moveToThread(&m_thread);
//    connect(&m_thread, &QThread::finished, m_holder.get(), &QObject::deleteLater);
    m_thread.start();
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_qthread(m_thread);

    connect(
        this, &VideoWidget::internal_set_resolution,
        m_holder.get(), &CameraHolder::set_resolution
    );
}
VideoWidget::~VideoWidget(){
    m_holder.reset();
    m_thread.quit();
    m_thread.wait();
}
QSize VideoWidget::current_resolution() const{
    if (!m_holder){
        return QSize();
    }
    return m_holder->current_resolution();
}
std::vector<QSize> VideoWidget::supported_resolutions() const{
    if (!m_holder){
        return std::vector<QSize>();
    }
    return m_holder->supported_resolutions();
}
void VideoWidget::set_resolution(const QSize& size){
    emit internal_set_resolution(size);
}
VideoSnapshot VideoWidget::snapshot(){
    if (!m_holder){
        return VideoSnapshot{QImage(), current_time()};
    }
    return m_holder->snapshot();
}
void VideoWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
    if (m_holder == nullptr){
        return;
    }
    m_camera_view->setFixedSize(this->size());
}





}
}
#endif
