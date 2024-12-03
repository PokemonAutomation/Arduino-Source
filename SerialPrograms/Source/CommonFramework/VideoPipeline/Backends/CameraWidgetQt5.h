/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt5VideoWidget_H
#define PokemonAutomation_VideoPipeline_Qt5VideoWidget_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <set>
#include <condition_variable>
#include <QThread>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QVideoProbe>
#include "Common/Cpp/EventRateTracker.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/UI/VideoWidget.h"
#include "CameraImplementations.h"
#include "VideoToolsQt5.h"

namespace PokemonAutomation{
namespace CameraQt5QCameraViewfinder{

class VideoWidget;


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<PokemonAutomation::CameraSession> make_camera(Logger& logger, Resolution default_resolution) const override;
};




class CameraSession : public QObject, public PokemonAutomation::CameraSession{
public:
    virtual void add_state_listener(StateListener& listener) override;
    virtual void remove_state_listener(StateListener& listener) override;

    virtual void add_frame_listener(VideoFrameListener& listener) override;
    virtual void remove_frame_listener(VideoFrameListener& listener) override;


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

    virtual double fps_source() override;
    virtual double fps_display() override;

    QVideoFrame latest_frame();

    virtual PokemonAutomation::VideoWidget* make_QtWidget(QWidget* parent) override;


private:
    //  These must be run on the UI thread.
    void shutdown();
    void startup();


private:
    //  All of these must be called under the lock.
    VideoSnapshot direct_snapshot_image();
    QImage direct_snapshot_probe(bool flip_vertical);

    VideoSnapshot snapshot_image();
    VideoSnapshot snapshot_probe();

    bool determine_frame_orientation();


private:
    friend class VideoWidget;

    Logger& m_logger;
    Resolution m_default_resolution;

    //  If you need both locks, acquire "m_lock" first.
    mutable std::mutex m_lock;
    SpinLock m_frame_lock;

    CameraInfo m_device;
    Resolution m_resolution;

    QCamera* m_camera = nullptr;
    std::unique_ptr<CameraScreenshotter> m_screenshotter;

    size_t m_max_frame_rate;
    std::chrono::milliseconds m_frame_period;
    std::vector<Resolution> m_resolutions;

    EventRateTracker m_fps_tracker;

//    SpinLock m_capture_lock;
    QVideoProbe* m_probe = nullptr;
    WallClock m_last_orientation_attempt;
    bool m_orientation_known = false;
//    bool m_use_probe_frames = false;
    bool m_flip_vertical = false;

    //  Last Frame
    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
//    QImage m_last_image;
//    WallClock m_last_image_timestamp;
    VideoSnapshot m_last_snapshot;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;

    std::set<StateListener*> m_listeners;

    LifetimeSanitizer m_sanitizer;
};



class VideoWidget : public PokemonAutomation::VideoWidget, private CameraSession::StateListener{
public:
    VideoWidget(QWidget* parent, CameraSession& session);
    virtual ~VideoWidget();

    virtual PokemonAutomation::CameraSession& camera() override{ return m_session; }

private:
    virtual void pre_shutdown() override;
    virtual void post_new_source(const CameraInfo& device, Resolution resolution) override;

//    virtual void resizeEvent(QResizeEvent* event) override;

private:
    CameraSession& m_session;
    QCameraViewfinder* m_camera_view = nullptr;
};








}
}
#endif
#endif
