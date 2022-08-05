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
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/UI/VideoWidget.h"
#include "CameraImplementations.h"
#include "VideoToolsQt5.h"

namespace PokemonAutomation{
namespace CameraQt5QCameraViewfinder{


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
};


class Camera : public QObject, public PokemonAutomation::Camera{
//public:
//    struct Listener{
//        virtual void new_frame_available() = 0;
//    };

//    void add_listener(Listener& listener);
//    void remove_listener(Listener& listener);

public:
    Camera(
        Logger& logger,
        const CameraInfo& info, const Resolution& desired_resolution
    );
    virtual ~Camera();

public:
    //  These are all thread-safe.

    virtual Resolution current_resolution() const override;
    virtual std::vector<Resolution> supported_resolutions() const override;
    virtual void set_resolution(const Resolution& size) override;

    virtual VideoSnapshot snapshot() override;

private:
    //  All of these must be called under the lock.
    VideoSnapshot direct_snapshot_image();
    QImage direct_snapshot_probe(bool flip_vertical);

    VideoSnapshot snapshot_image();
    VideoSnapshot snapshot_probe();

    bool determine_frame_orientation();

private:
    friend class VideoWidget2;

    Logger& m_logger;
    QCamera* m_camera = nullptr;
    CameraScreenshotter m_screenshotter;

    size_t m_max_frame_rate;
    std::chrono::milliseconds m_frame_period;
    std::vector<Resolution> m_resolutions;

    mutable std::mutex m_lock;
    Resolution m_resolution;

//    SpinLock m_capture_lock;
    QVideoProbe* m_probe = nullptr;
    WallClock m_last_orientation_attempt;
    bool m_orientation_known = false;
//    bool m_use_probe_frames = false;
    bool m_flip_vertical = false;

    SpinLock m_frame_lock;

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

//    std::set<Listener*> m_listeners;
};




class VideoWidget2 : public PokemonAutomation::VideoWidget{
public:
    VideoWidget2(QWidget* parent, Camera& camera);
    virtual ~VideoWidget2();

    virtual Camera& camera() override{ return m_camera; }

private:
    Camera& m_camera;
    QCameraViewfinder* m_camera_view = nullptr;
};










}
}
#endif
#endif
