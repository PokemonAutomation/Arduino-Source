/*  Camera Widget (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6 && QT_VERSION_MINOR >= 5

#include <chrono>
#include <iostream>
//#include <QThread>
#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QImageCapture>
//#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Time.h"
//#include "Common/Cpp/PrettyPrint.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetQt6.5.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace CameraQt65QMediaCaptureSession{






std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    std::vector<CameraInfo> ret;
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            return camera.description().toStdString();
        }
    }
    std::cout << "Error: no such camera for CameraInfo: " << info.device_name() << std::endl;
    return "";
}
std::unique_ptr<VideoSource> CameraBackend::make_video_source(
    Logger& logger,
    const CameraInfo& info,
    Resolution resolution
) const{
    return std::make_unique<CameraVideoSource>(logger, info, resolution);
}







CameraVideoSource::~CameraVideoSource(){
    if (!m_capture_session){
        return;
    }
    try{
        m_logger.log("Stopping Camera...");
    }catch (...){}

    m_camera->stop();
    m_capture_session.reset();
    m_camera.reset();
}
CameraVideoSource::CameraVideoSource(
    Logger& logger,
    const CameraInfo& info,
    Resolution desired_resolution
)
    : VideoSource(true)
    , m_logger(logger)
    , m_last_image_timestamp(WallClock::min())
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
    , m_last_frame_seqnum(0)
{
    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt65QMediaCaptureSession");

    auto cameras = QMediaDevices::videoInputs();
    const QCameraDevice* device = nullptr;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            device = &camera;
            break;
        }
    }
    if (device == nullptr){
        m_logger.log("Camera not found: " + info.device_name(), COLOR_RED);
        return;
    }
    m_logger.log("Camera: " + device->description().toStdString());

    QList<QCameraFormat> formats = device->videoFormats();
    if (formats.empty()){
        m_logger.log("No usable resolutions: " + device->description().toStdString(), COLOR_RED);
        return;
    }

    std::map<Resolution, const QCameraFormat*> resolution_map;
    for (const QCameraFormat& format : formats){
        QSize resolution = format.resolution();
        resolution_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(resolution.width(), resolution.height()),
            std::forward_as_tuple(&format)
        );
    }

    const QCameraFormat* format = nullptr;
    m_resolutions.clear();
    for (const auto& res : resolution_map){
        m_resolutions.emplace_back(res.first);
        if (res.first == desired_resolution){
            format = res.second;
        }
    }
    if (format == nullptr){
        format = resolution_map.rbegin()->second;
    }

    QSize size = format->resolution();
    m_resolution = Resolution(size.width(), size.height());
    m_logger.log("Resolution: " + m_resolution.to_string());

    m_camera.reset(new QCamera(*device));
    m_camera->setCameraFormat(*format);

    m_capture_session.reset(new QMediaCaptureSession());
    m_capture_session->setCamera(m_camera.get());

    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() == QCamera::NoError){
            return;
        }
        m_logger.log("QCamera error: " + m_camera->errorString().toStdString(), COLOR_RED);
    });

    m_camera->start();
}


void CameraVideoSource::set_video_output(QGraphicsVideoItem& item){
    if (m_capture_session == nullptr){
        return;
    }
    if (m_capture_session->videoSink() == item.videoSink()){
        return;
    }
    m_capture_session->setVideoOutput(&item);

    connect(
        item.videoSink(), &QVideoSink::videoFrameChanged,
        m_camera.get(), [&](const QVideoFrame& frame){
            //  This will be on the main thread. So we waste as little time as
            //  possible. Shallow-copy the frame, update the listeners, and
            //  return immediately to unblock the main thread.

            WallClock now = current_time();
            {
                WriteSpinLock lg(m_frame_lock);

                //  Skip duplicate frames.
                if (frame.startTime() != -1 && frame.startTime() <= m_last_frame.startTime()){
                    return;
                }

                m_last_frame = frame;
                m_last_frame_timestamp = now;
                uint64_t seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
                seqnum++;
                m_last_frame_seqnum.store(seqnum, std::memory_order_relaxed);
            }
            report_source_frame(std::make_shared<VideoFrame>(now, frame));
        },
        Qt::DirectConnection
    );
}





VideoSnapshot CameraVideoSource::snapshot(){
    //  This will be coming in from random threads. (not the main thread)
    //  So we efficiently grab the last frame to unblock the main thread.
    //  Then we can do any expensive post-processing as needed.

    std::lock_guard<std::mutex> lg(m_cache_lock);

    //  Check the cached image frame. If it's not stale, return it immediately.
    uint64_t frame_seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
    if (!m_last_image.isNull() && m_last_image_seqnum == frame_seqnum){
        return VideoSnapshot(m_last_image, m_last_image_timestamp);
    }

    //  Cached image is stale. Grab the latest frame.
    QVideoFrame frame;
    WallClock frame_timestamp;
    {
        ReadSpinLock lg0(m_frame_lock);
        frame_seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
        frame = m_last_frame;
        frame_timestamp = m_last_frame_timestamp;
    }

    if (!frame.isValid()){
        m_logger.log("QVideoFrame is null.", COLOR_RED);
        return VideoSnapshot();
    }

    //  Converting the QVideoFrame to QImage is expensive. Time it and
    //  report performance.
    WallClock time0 = current_time();

    QImage image = frame.toImage();
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    WallClock time1 = current_time();
    const int64_t duration = std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
    m_stats_conversion.report_data(m_logger, uint32_t(duration));

    //  Update the cached image.
    m_last_image = std::move(image);
    m_last_image_timestamp = frame_timestamp;
    m_last_image_seqnum = frame_seqnum;

    return VideoSnapshot(m_last_image, m_last_image_timestamp);
}

QWidget* CameraVideoSource::make_display_QtWidget(QWidget* parent){
    return new CameraVideoDisplay(parent, *this);
}








CameraVideoDisplay::CameraVideoDisplay(QWidget* parent, CameraVideoSource& source)
    : QWidget(parent)
    , m_source(source)
    , m_view(new StaticQGraphicsView(this))
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    m_view->setFixedSize(this->size());
    m_view->setScene(&m_scene);
    m_video.setSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_scene.addItem(&m_video);
    source.set_video_output(m_video);

    connect(
        &m_scene, &QGraphicsScene::changed,
        this, [&](const QList<QRectF>&){
            auto scope_check = m_sanitizer.check_scope();
            m_source.report_rendered_frame(current_time());
        }
    );
}
void CameraVideoDisplay::resizeEvent(QResizeEvent* event){
    auto scope_check = m_sanitizer.check_scope();
    m_view->setFixedSize(this->size());
    m_scene.setSceneRect(QRectF(QPointF(0, 0), this->size()));
    m_video.setSize(this->size());
}















}
}
#endif
