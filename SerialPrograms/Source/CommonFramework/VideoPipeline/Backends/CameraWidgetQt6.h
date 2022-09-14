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
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/VideoPipeline/UI/VideoWidget.h"
#include "CameraImplementations.h"

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<PokemonAutomation::CameraSession> make_camera(Logger& logger, Resolution default_resolution) const override;
};


struct FrameListener{
    virtual void new_frame_available() = 0;
};



class CameraSession : public QObject, public PokemonAutomation::CameraSession{
public:
    virtual void add_listener(Listener& listener) override;
    virtual void remove_listener(Listener& listener) override;
    void add_listener(FrameListener& listener);
    void remove_listener(FrameListener& listener);


public:
    virtual ~CameraSession();
    CameraSession(Logger& logger, Resolution default_resolution);

    virtual void get(CameraOption& option) override;
    virtual void set(const CameraOption& option) override;

    virtual void reset() override;
    virtual void set_source(CameraInfo device) override;
    virtual void set_resolution(Resolution resolution) override;

    virtual CameraInfo current_device() const override;
    virtual Resolution current_resolution() const override;
    virtual std::vector<Resolution> supported_resolutions() const override;

    virtual VideoSnapshot snapshot() override;

    QVideoFrame latest_frame();

    virtual VideoWidget* make_QtWidget(QWidget* parent) override;


private:
    //  These must be run on the UI thread.
    void shutdown();
    void startup();


private:
    Logger& m_logger;
    Resolution m_default_resolution;

    //  If you need both locks, acquire "m_lock" first.
    mutable std::mutex m_lock;
    SpinLock m_frame_lock;

    CameraInfo m_device;
    Resolution m_resolution;

    QList<QCameraFormat> m_formats;
    std::map<Resolution, const QCameraFormat*> m_resolution_map;

    std::unique_ptr<QCamera> m_camera;
    std::unique_ptr<QVideoSink> m_video_sink;
    std::unique_ptr<QMediaCaptureSession> m_capture;

    std::vector<Resolution> m_resolutions;

    //  Last Frame
    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
    QImage m_last_image;
    WallClock m_last_image_timestamp;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;

    std::set<Listener*> m_ui_listeners;
    std::set<FrameListener*> m_frame_listeners;

    LifetimeSanitizer m_sanitizer;
};




class VideoWidget : public PokemonAutomation::VideoWidget, public FrameListener{
public:
    VideoWidget(QWidget* parent, CameraSession& session);
    virtual ~VideoWidget();

    virtual PokemonAutomation::CameraSession& camera() override{ return m_session; }

private:
    virtual void new_frame_available() override;
    virtual void paintEvent(QPaintEvent*) override;


private:
    CameraSession& m_session;
};









}
}
#endif
#endif
