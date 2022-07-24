/*  Video Widget (Custom Frame)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <QCoreApplication>
#include <QCameraInfo>
#include <QVBoxLayout>
#include <QPainter>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CameraWidgetQt5CustomFrame.h"

namespace PokemonAutomation{
namespace CameraQt5CustomFrame{



std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
    return qt5_get_all_cameras();
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
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





CameraModeLegacy::CameraModeLegacy(LoggerQt& logger, QCamera& camera)
    : m_screenshotter(logger, camera)
{
    QCameraViewfinderSettings settings = camera.viewfinderSettings();
    int max_frame_rate = settings.maximumFrameRate();
    m_frame_period = std::chrono::milliseconds(25);
    if (0 < max_frame_rate){
        m_frame_period = std::chrono::milliseconds((uint64_t)(1.0 / max_frame_rate * 1000));
    }
}
VideoSnapshot CameraModeLegacy::snapshot(){
    //  Only allow one snapshot at a time.
    std::unique_lock<std::mutex> lg1(m_snapshot_lock);

    WallClock timestamp = current_time();
    if (m_last_snapshot.timestamp + m_frame_period > timestamp){
        return m_last_snapshot;
    }

    m_last_snapshot = m_screenshotter.snapshot();
    return m_last_snapshot;
}



CameraModeProbeExpiration::CameraModeProbeExpiration(LoggerQt& logger, QCamera& camera, QVideoProbe& probe)
    : m_screenshotter(logger, camera)
    , m_probe(probe)
    , m_last_frame_seqnum(0)
    , m_last_image_seqnum(0)
{
    QCameraViewfinderSettings settings = camera.viewfinderSettings();
    int max_frame_rate = settings.maximumFrameRate();
    m_frame_period = std::chrono::milliseconds(25);
    if (0 < max_frame_rate){
        m_frame_period = std::chrono::milliseconds((uint64_t)(1.0 / max_frame_rate * 1000));
    }

    connect(
        &m_probe, &QVideoProbe::videoFrameProbed,
        this, [=](const QVideoFrame& frame){
            uint64_t seqnum = m_last_frame_seqnum.load(std::memory_order_acquire);
            m_last_frame_seqnum.store(seqnum + 1, std::memory_order_release);
        },
        Qt::DirectConnection
    );
}
VideoSnapshot CameraModeProbeExpiration::snapshot(){
    //  Only allow one snapshot at a time.
    std::unique_lock<std::mutex> lg1(m_snapshot_lock);

    uint64_t frame_seqnum = m_last_frame_seqnum.load(std::memory_order_acquire);
    if (m_last_image_seqnum == frame_seqnum){
        return m_last_snapshot;
    }

    m_last_snapshot = m_screenshotter.snapshot();
    m_last_image_seqnum = frame_seqnum;
    return m_last_snapshot;
}




CameraModeSelfRender::CameraModeSelfRender(LoggerQt& logger, VideoWidget& widget, QVideoProbe& probe, bool flip_vertical)
    : m_logger(logger)
    , m_probe(probe)
    , m_stopping(false)
    , m_flip_vertical(flip_vertical)
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
    , m_thread(&CameraModeSelfRender::thread_loop, this)
{
    connect(
        &m_probe, &QVideoProbe::videoFrameProbed,
        this, [=](const QVideoFrame& frame){
            WallClock now = current_time();
            std::lock_guard<std::mutex> lg(m_lock);
            m_last_frame = frame;
            m_last_frame_timestamp = now;
            m_last_frame_seqnum++;
            m_cv.notify_all();
        },
        Qt::DirectConnection
    );
    connect(
        this, &CameraModeSelfRender::new_frame,
        &widget, &VideoWidget::new_frame
    );
}
CameraModeSelfRender::~CameraModeSelfRender(){
    {
        std::unique_lock<std::mutex> m_lock;
        m_stopping.store(true, std::memory_order_release);
        m_cv.notify_all();
    }
    m_thread.join();
}
void CameraModeSelfRender::set_widget_dimensions(QSize size){
    m_widget_width.store(size.width(), std::memory_order_release);
    m_widget_height.store(size.height(), std::memory_order_release);
}
VideoSnapshot CameraModeSelfRender::snapshot(){
    std::lock_guard<std::mutex> lg(m_lock);
    VideoSnapshot ret;
    snapshot(ret);
    return ret;
}
QImage CameraModeSelfRender::get_display_image(){
    std::lock_guard<std::mutex> lg(m_lock);
    return m_last_display;
}
uint64_t CameraModeSelfRender::snapshot(VideoSnapshot& image){
    if (m_last_image && m_last_image_seqnum == m_last_frame_seqnum){
        image = m_last_image;
        return m_last_frame_seqnum;
    }

    WallClock time0 = current_time();

    QImage current_image = frame_to_image(m_logger, m_last_frame, m_flip_vertical);
    if (current_image.isNull()){
        image = m_last_image;
        return m_last_frame_seqnum;
    }
    m_last_image = VideoSnapshot(
        std::move(current_image),
        m_last_frame_timestamp
    );
    m_last_image_seqnum = m_last_frame_seqnum;

    WallClock time1 = current_time();
    m_stats_conversion.report_data(m_logger, std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count());

    image = m_last_image;
    return m_last_frame_seqnum;
}
void CameraModeSelfRender::thread_loop(){
    std::unique_lock<std::mutex> lg(m_lock);
    while (true){
        if (m_stopping.load(std::memory_order_acquire)){
            return;
        }

        VideoSnapshot frame;
        uint64_t seqnum = snapshot(frame);

        QSize widget_size(
            m_widget_width.load(std::memory_order_acquire),
            m_widget_height.load(std::memory_order_acquire)
        );
#if 1
        if (seqnum != m_last_display_seqnum || m_last_display.size() != widget_size){
            m_last_display = frame.frame->to_QImage_ref().scaled(widget_size);
            m_last_display_seqnum = seqnum;
            emit new_frame();
        }
#endif
        m_cv.wait(lg);
    }
}


















CameraHolder::CameraHolder(
    LoggerQt& logger, VideoWidget& widget,
    const CameraInfo& info, const QSize& desired_resolution
)
    : m_logger(logger)
{
    m_camera = new QCamera(QCameraInfo(info.device_name().c_str()), this);

    m_probe = new QVideoProbe(m_camera);
    if (!m_probe->setSource(m_camera)){
        logger.log("Unable to initialize QVideoProbe() capture.", COLOR_RED);
        delete m_probe;
        m_probe = nullptr;
    }

    m_camera->start();

    for (const auto& size : m_camera->supportedViewfinderResolutions()){
        m_supported_resolutions.emplace_back(size);
    }

    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
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

    connect(
        this, &CameraHolder::use_view_finder,
        &widget, &VideoWidget::use_view_finder,
        Qt::ConnectionType::QueuedConnection
    );

//    m_mode.reset(new CameraModeLegacy(logger, *m_camera));
//    m_mode.reset(new CameraModeProbeExpiration(logger, *m_camera, *m_probe));
    m_mode.reset(new CameraModeSelfRender(logger, widget, *m_probe, false));
    emit use_view_finder(false);
}
CameraHolder::~CameraHolder(){
    m_camera->stop();
    m_mode.reset();
}
void CameraHolder::set_resolution(const QSize& size){
    std::unique_lock<std::mutex> lg(m_lock);
    QCameraViewfinderSettings settings = m_camera->viewfinderSettings();
    if (settings.resolution() == size){
        return;
    }
    settings.setResolution(size);
    m_camera->setViewfinderSettings(settings);
    m_current_resolution = size;
}
void CameraHolder::set_view_finder(QCameraViewfinder* widget){
    m_camera->setViewfinder(widget);
}
void CameraHolder::set_widget_dimensions(QSize size){
    std::unique_lock<std::mutex> lg(m_lock);
    m_mode->set_widget_dimensions(size);
}
VideoSnapshot CameraHolder::snapshot(){
    std::unique_lock<std::mutex> lg(m_lock);
    return m_mode->snapshot();
}
QImage CameraHolder::get_display_image(){
    std::unique_lock<std::mutex> lg(m_lock);
    return m_mode->get_display_image();
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
    m_holder->moveToThread(&m_thread);
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
    if (m_holder == nullptr){
        return QSize();
    }
    return m_holder->current_resolution();
}
std::vector<QSize> VideoWidget::supported_resolutions() const{
    if (m_holder == nullptr){
        return std::vector<QSize>();
    }
    return m_holder->supported_resolutions();
}
void VideoWidget::set_resolution(const QSize& size){
    if (m_holder == nullptr){
        return;
    }
    emit internal_set_resolution(size);
}
VideoSnapshot VideoWidget::snapshot(){
    if (m_holder == nullptr){
        return VideoSnapshot{QImage(), current_time()};
    }
    return m_holder->snapshot();
}
void VideoWidget::use_view_finder(bool enabled){
    if (enabled){
        if (m_camera_view == nullptr){
//            cout << "camera view - on" << endl;
            m_camera_view = new QCameraViewfinder(this);
            layout()->addWidget(m_camera_view);
            m_camera_view->setMinimumSize(80, 45);
            m_holder->set_view_finder(m_camera_view);
        }
    }else{
        if (m_camera_view != nullptr){
//            cout << "camera view - off" << endl;
            m_holder->set_view_finder(nullptr);
            delete m_camera_view;
            m_camera_view = nullptr;
        }
    }
}
void VideoWidget::new_frame(){
    QWidget::update();
}
void VideoWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
    if (m_holder == nullptr){
        return;
    }
    if (m_camera_view){
        m_camera_view->setFixedSize(this->size());
    }else{
        m_holder->set_widget_dimensions(this->size());
    }
}
void VideoWidget::paintEvent(QPaintEvent* event){
    PokemonAutomation::VideoWidget::paintEvent(event);

    if (m_holder == nullptr){
        return;
    }
    if (m_camera_view != nullptr){
        return;
    }
    QImage image = m_holder->get_display_image();
    if (image.isNull()){
        return;
    }
    if (image.size() != this->size()){
        image = image.scaled(this->size());
    }

    QPainter painter(this);
    painter.fillRect(rect(), image);
}

























}
}
#endif
