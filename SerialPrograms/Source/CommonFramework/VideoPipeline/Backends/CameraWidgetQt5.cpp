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
#include "CommonFramework/VideoPipeline/CameraOption.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
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
std::unique_ptr<PokemonAutomation::CameraSession> CameraBackend::make_camera(Logger& logger, Resolution default_resolution) const{
    return std::make_unique<CameraSession>(logger, default_resolution);
}




void CameraSession::add_state_listener(StateListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void CameraSession::remove_state_listener(StateListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
}
void CameraSession::add_frame_listener(VideoFrameListener& listener){
    auto scope_check = m_sanitizer.check_scope();

}
void CameraSession::remove_frame_listener(VideoFrameListener& listener){
    auto scope_check = m_sanitizer.check_scope();

}

CameraSession::~CameraSession(){
    shutdown();
}
CameraSession::CameraSession(Logger& logger, Resolution default_resolution)
    : m_logger(logger)
    , m_default_resolution(default_resolution)
    , m_last_orientation_attempt(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{}

void CameraSession::get(CameraOption& option){
    std::lock_guard<std::mutex> lg(m_lock);
    option.info = m_device;
    option.current_resolution = m_resolution;
}
void CameraSession::set(const CameraOption& option){
    QMetaObject::invokeMethod(this, [this, option]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        m_device = option.info;
        m_resolution = option.current_resolution;
        startup();
    });
}
void CameraSession::reset(){
    QMetaObject::invokeMethod(this, [this]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        startup();
    });
}
void CameraSession::set_source(CameraInfo device){
//    cout << "CameraSession::set_source()" << endl;
    QMetaObject::invokeMethod(this, [this, device]{
        std::lock_guard<std::mutex> lg(m_lock);
        shutdown();
        m_device = std::move(device);
        startup();
    });
}
void CameraSession::set_resolution(Resolution resolution){
    QMetaObject::invokeMethod(this, [this, resolution]{
        std::lock_guard<std::mutex> lg(m_lock);
        if (!m_camera){
            m_resolution = resolution;
            return;
        }
        m_logger.log("Setting resolution to: " + resolution.to_string());
        bool found = false;
        for (const Resolution& res : m_resolutions){
            if (res == resolution){
                found = true;
                break;
            }
        }
        if (!found){
            m_logger.log("Resolution not supported.", COLOR_RED);
            return;
        }
        for (StateListener* listener : m_listeners){
            listener->pre_resolution_change(resolution);
        }
        m_resolution = resolution;
        QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
        settings.setResolution(QSize((int)resolution.width, (int)resolution.height));
        m_camera->setViewfinderSettings(settings);
        for (StateListener* listener : m_listeners){
            listener->post_resolution_change(resolution);
        }
    });
}
CameraInfo CameraSession::current_device() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_device;
}
Resolution CameraSession::current_resolution() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_resolution;
}
std::vector<Resolution> CameraSession::supported_resolutions() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_resolutions;
}



VideoSnapshot CameraSession::direct_snapshot_image(){
//    std::unique_lock<std::mutex> lg(m_lock);
    if (m_camera == nullptr){
        return VideoSnapshot();
    }
    return m_screenshotter->snapshot();
}
QImage CameraSession::direct_snapshot_probe(bool flip_vertical){
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
VideoSnapshot CameraSession::snapshot_image(){
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

    m_last_snapshot = m_screenshotter->snapshot();
    m_last_image_seqnum = frame_seqnum;
    return m_last_snapshot;
}
VideoSnapshot CameraSession::snapshot_probe(){
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
bool CameraSession::determine_frame_orientation(){
    //  Qt 5.12 is really shitty in that there's no way to figure out the
    //  orientation of a QVideoFrame. So here we'll try to figure it out
    //  the poor man's way. Snapshot using both QCameraImageCapture and
    //  QVideoProbe and compare them.

    //  This function cannot be called on the UI thread.
    //  This function must be called under the lock.

    VideoSnapshot reference = direct_snapshot_image();
    QImage frame = direct_snapshot_probe(false);
    m_orientation_known = PokemonAutomation::determine_frame_orientation(m_logger, reference, frame, m_flip_vertical);
    return m_orientation_known;
}
VideoSnapshot CameraSession::snapshot(){
    std::unique_lock<std::mutex> lg(m_lock);

    //  Frame screenshots are disabled.
    if (!GlobalSettings::instance().VIDEO_PIPELINE->ENABLE_FRAME_SCREENSHOTS){
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
double CameraSession::fps_source(){
    SpinLockGuard lg(m_frame_lock);
    return m_fps_tracker.events_per_second();
}
double CameraSession::fps_display(){
    return -1;
}




void CameraSession::shutdown(){
    if (m_camera == nullptr){
        return;
    }

    m_logger.log("Stopping Camera...");

    m_camera->stop();
    for (StateListener* listener : m_listeners){
        listener->pre_shutdown();
    }

    m_screenshotter.reset();

    delete m_camera;
    m_camera = nullptr;
    m_probe = nullptr;

    m_last_frame = QVideoFrame();
    m_last_frame_timestamp = current_time();
    m_last_frame_seqnum++;

    {
        SpinLockGuard lg(m_frame_lock);

        m_last_frame = QVideoFrame();
        m_last_frame_timestamp = current_time();
        m_last_frame_seqnum++;

        m_last_image_seqnum = m_last_frame_seqnum;
    }

    for (StateListener* listener : m_listeners){
        listener->post_shutdown();
    }
}
void CameraSession::startup(){
    if (!m_device){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt5QCameraViewfinder");

    {
        QByteArray data = QByteArray::fromStdString(m_device.device_name());
        m_camera = new QCamera(QCameraInfo(data), this);
    }

    //  Setup the screenshot capture.
    m_probe = new QVideoProbe(m_camera);
#if 1
    if (!m_probe->setSource(m_camera)){
//        m_use_probe_frames = false;
        m_logger.log("Unable to initialize QVideoProbe() capture.", COLOR_RED);
        delete m_probe;
        m_probe = nullptr;
    }
#endif

    m_camera->start();

    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    m_max_frame_rate = settings.maximumFrameRate();
    m_frame_period = std::chrono::milliseconds(25);
    if (0 < m_max_frame_rate){
        m_frame_period = std::chrono::milliseconds((uint64_t)(1.0 / m_max_frame_rate * 1000));
    }

    bool default_found = false;
    bool desired_found = false;
    for (const auto& size : m_camera->supportedViewfinderResolutions()){
        Resolution resolution = Resolution(size.width(), size.height());
        m_resolutions.emplace_back(resolution);
        default_found |= resolution == m_default_resolution;
        desired_found |= resolution == m_resolution;
    }

    if (desired_found){
        settings.setResolution(QSize((int)m_resolution.width, (int)m_resolution.height));
        m_camera->setViewfinderSettings(settings);
    }else if (default_found){
        settings.setResolution(QSize((int)m_default_resolution.width, (int)m_default_resolution.height));
        m_camera->setViewfinderSettings(settings);
        m_resolution = m_default_resolution;
    }else{
        QSize resolution = settings.resolution();
        m_resolution = Resolution(resolution.width(), resolution.height());
    }

    m_screenshotter.reset(new CameraScreenshotter(m_logger, *m_camera));

    if (m_probe){
        connect(
            m_probe, &QVideoProbe::videoFrameProbed,
            m_camera, [this](const QVideoFrame& frame){
                WallClock now = current_time();
                SpinLockGuard lg(m_frame_lock);
                if (GlobalSettings::instance().VIDEO_PIPELINE->ENABLE_FRAME_SCREENSHOTS){
                    m_last_frame = frame;
                }
                m_last_frame_timestamp = now;
                m_last_frame_seqnum++;
                m_fps_tracker.push_event(now);
            },
            Qt::DirectConnection
        );
    }

    for (StateListener* listener : m_listeners){
        listener->post_new_source(m_device, m_resolution);
    }
}

PokemonAutomation::VideoWidget* CameraSession::make_QtWidget(QWidget* parent){
    return new VideoWidget(parent, *this);
}






VideoWidget::VideoWidget(QWidget* parent, CameraSession& session)
    : PokemonAutomation::VideoWidget(parent)
    , m_session(session)
{
    this->setMinimumSize(80, 45);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);

    //  Setup the video display.
    m_camera_view = new QCameraViewfinder(this);
    layout->addWidget(m_camera_view);
    m_camera_view->setMinimumSize(80, 45);
    m_camera_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (session.m_camera){
        session.m_camera->setViewfinder(m_camera_view);
    }

    session.add_state_listener(*this);
}
VideoWidget::~VideoWidget(){
    m_session.add_state_listener(*this);
    if (m_session.m_camera){
        m_session.m_camera->setViewfinder((QCameraViewfinder*)nullptr);
    }
}


void VideoWidget::pre_shutdown(){
//    cout << "VideoWidget::shutdown() - start" << endl;
    if (m_session.m_camera){
        m_session.m_camera->setViewfinder((QCameraViewfinder*)nullptr);
    }
//    cout << "VideoWidget::shutdown() - end" << endl;
}
void VideoWidget::post_new_source(const CameraInfo& device, Resolution resolution){
//    cout << "VideoWidget::new_source" << endl;
    m_session.m_camera->setViewfinder(m_camera_view);
}
//void VideoWidget::resizeEvent(QResizeEvent* event){
//    cout << "VideoWidget: " << this->width() << " x " << this->height() << endl;
//    cout << "Viewfinder: " << m_camera_view->width() << " x " << m_camera_view->height() << endl;
//}












}
}
#endif

