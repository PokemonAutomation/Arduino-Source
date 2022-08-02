/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt6VideoWidget_H
#define PokemonAutomation_VideoPipeline_Qt6VideoWidget_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <set>
#include <mutex>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CameraInfo.h"
#include "CameraImplementations.h"
#include "VideoWidget.h"

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<Camera> make_camera(
        Logger& logger,
        const CameraInfo& info,
        const Resolution& desired_resolution
    ) const override;

    virtual VideoWidget* make_video_widget(QWidget* parent, Camera& camera) const override;

    virtual VideoWidget* make_video_widget(
        QWidget& parent,
        Logger& logger,
        const CameraInfo& info,
        const Resolution& desired_resolution
    ) const override;
};


class Camera : public QObject, public PokemonAutomation::Camera{
public:
    struct Listener{
        virtual void new_frame_available() = 0;
    };

public:
    Camera(
        Logger& logger,
        const CameraInfo& info, const Resolution& desired_resolution
    );
    virtual ~Camera();

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

public:
    //  These are all thread-safe.

    virtual Resolution current_resolution() const override;
    virtual std::vector<Resolution> supported_resolutions() const override;
    virtual void set_resolution(const Resolution& size) override;

    QVideoFrame latest_frame();
    virtual VideoSnapshot snapshot() override;

private:
    friend class VideoWidget2;

    Logger& m_logger;
    std::vector<Resolution> m_resolutions;

    QCameraDevice m_info;
    QCamera* m_camera = nullptr;
    QVideoSink* m_videoSink = nullptr;
    std::vector<QCameraFormat> m_formats;

    //  If you need both locks, acquire "m_lock" first.
    mutable std::mutex m_lock;
    SpinLock m_frame_lock;

    //  Last Frame
    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
    QImage m_last_image;
    WallClock m_last_image_timestamp;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;

    std::set<Listener*> m_listeners;

    LifetimeSanitizer m_sanitizer;
};


class VideoWidget2 : public PokemonAutomation::VideoWidget, public Camera::Listener{
public:
    VideoWidget2(QWidget* parent, Camera& camera);
    VideoWidget2(
        QWidget* parent,
        Logger& logger,
        const CameraInfo& info, const Resolution& desired_resolution
    );
    virtual ~VideoWidget2();

    virtual Camera& camera() override{ return m_camera; }

private:
    virtual void new_frame_available() override;
    virtual void paintEvent(QPaintEvent*) override;


private:
    std::unique_ptr<Camera> m_backing;
    Camera& m_camera;
    QMediaCaptureSession m_captureSession;
};










}
}
#endif
#endif
