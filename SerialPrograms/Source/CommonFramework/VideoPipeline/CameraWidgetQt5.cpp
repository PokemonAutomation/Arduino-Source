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
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoToolsQt5.h"
#include "CameraWidgetQt5.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt5QCameraViewfinder{



std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
    return qt5_get_all_cameras();
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
    return qt5_get_camera_name(info);
}
std::unique_ptr<PokemonAutomation::Camera> CameraBackend::make_camera(
    Logger& logger,
    const CameraInfo& info,
    const QSize& desired_resolution
) const{
    return std::make_unique<Camera>(logger, info, desired_resolution);
}
VideoWidget* CameraBackend::make_video_widget(QWidget& parent, PokemonAutomation::Camera& camera) const{
    Camera* casted = dynamic_cast<Camera*>(&camera);
    if (casted == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching camera session/widget types.");
    }
    return new VideoWidget2(&parent, *casted);
}
PokemonAutomation::VideoWidget* CameraBackend::make_video_widget(
    QWidget& parent,
    Logger& logger,
    const CameraInfo& info,
    const QSize& desired_resolution
) const{
    return new VideoWidget2(&parent, logger, info, desired_resolution);
}



Camera::Camera(
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : m_logger(logger)
    , m_camera(new QCamera(QCameraInfo(info.device_name().c_str()), this))
    , m_screenshotter(logger, *m_camera)
    , m_last_orientation_attempt(WallClock::min())
//    , m_use_probe_frames(true)
//    , m_flip_vertical(true)
    , m_last_frame_seqnum(0)
//    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{
    logger.log("Constructing VideoWidget: Backend = CameraQt5QCameraViewfinder");
    if (!info){
        return;
    }


    //  Setup the screenshot capture.
    m_probe = new QVideoProbe(this);
    if (!m_probe->setSource(m_camera)){
//        m_use_probe_frames = false;
        logger.log("Unable to initialize QVideoProbe() capture.", COLOR_RED);
        delete m_probe;
        m_probe = nullptr;
    }


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
            },
            Qt::DirectConnection
        );
    }
}
Camera::~Camera(){}

#if 0
void Camera::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void Camera::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}
#endif

QSize Camera::current_resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return QSize();
    }
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    return settings.resolution();
}
std::vector<QSize> Camera::supported_resolutions() const{
    return m_resolutions;
}
void Camera::set_resolution(const QSize& size){
    std::lock_guard<std::mutex> lg(m_lock);
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    if (settings.resolution() == size){
        return;
    }
    settings.setResolution(size);
    m_camera->setViewfinderSettings(settings);
    m_resolution = size;
}

VideoSnapshot Camera::direct_snapshot_image(){
//    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return VideoSnapshot();
    }
    return m_screenshotter.snapshot();
}
QImage Camera::direct_snapshot_probe(bool flip_vertical){
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
VideoSnapshot Camera::snapshot_image(){
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
        if (m_last_snapshot){
            if (m_probe){
                //  If we have the probe enabled, we know if a new frame has been pushed.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_image_seqnum == frame_seqnum){
//                    cout << "cached 0" << endl;
                    return m_last_snapshot;
                }
            }else{
                //  Otherwise, we have to use time.
//                cout << now - m_last_snapshot.load(std::memory_order_acquire) << endl;
                if (m_last_snapshot.timestamp + m_frame_period > now){
//                    cout << "cached 1" << endl;
                    return m_last_snapshot;
                }
            }
        }
    }

    WallClock time0 = current_time();

    m_last_snapshot = m_screenshotter.snapshot();
    m_last_image_seqnum = frame_seqnum;
    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());
    return m_last_snapshot;
}
VideoSnapshot Camera::snapshot_probe(){
//    cout << "snapshot_probe()" << endl;
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
        if (m_last_snapshot && m_last_image_seqnum == frame_seqnum){
            return m_last_snapshot;
        }
        frame = m_last_frame;
        frame_timestamp = m_last_frame_timestamp;
    }

    WallClock time0 = current_time();

    m_last_snapshot = VideoSnapshot(
        frame_to_image(m_logger, frame, m_flip_vertical),
        frame_timestamp
    );
    m_last_image_seqnum = frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    return m_last_snapshot;
}
bool Camera::determine_frame_orientation(){
    //  Qt 5.12 is really shitty in that there's no way to figure out the
    //  orientation of a QVideoFrame. So here we'll try to figure it out
    //  the poor man's way. Snapshot using both QCameraImageCapture and
    //  QVideoProbe and compare them.

    //  This function cannot be called on the UI thread.
    //  This function must be called under the lock.

    std::shared_ptr<const ImageRGB32> reference = direct_snapshot_image();
    QImage frame = direct_snapshot_probe(false);
    m_orientation_known = PokemonAutomation::determine_frame_orientation(m_logger, *reference, frame, m_flip_vertical);
    return m_orientation_known;
}
VideoSnapshot Camera::snapshot(){
    std::unique_lock<std::mutex> lg(m_lock);

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








VideoWidget2::VideoWidget2(QWidget* parent, Camera& camera)
    : PokemonAutomation::VideoWidget(parent)
    , m_camera(camera)
{
    if (m_camera.m_camera == nullptr){
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);


    //  Setup the video display.
    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_camera.m_camera->setViewfinder(m_camera_view);
}
VideoWidget2::VideoWidget2(
    QWidget* parent,
    Logger& logger,
    const CameraInfo& info, const QSize& desired_resolution
)
    : PokemonAutomation::VideoWidget(parent)
    , m_backing(new Camera(logger, info, desired_resolution))
    , m_camera(*m_backing)
{
    if (m_camera.m_camera == nullptr){
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);


    //  Setup the video display.
    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_camera.m_camera->setViewfinder(m_camera_view);

//    m_camera.add_listener(*this);
}
VideoWidget2::~VideoWidget2(){
//    m_camera.remove_listener(*this);
}

QSize VideoWidget2::current_resolution() const{
    return m_camera.current_resolution();
}
std::vector<QSize> VideoWidget2::supported_resolutions() const{
    return m_camera.supported_resolutions();
}
void VideoWidget2::set_resolution(const QSize& size){
    m_camera.set_resolution(size);
}
VideoSnapshot VideoWidget2::snapshot(){
    return m_camera.snapshot();
}






















}
}
#endif

