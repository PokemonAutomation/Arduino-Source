/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoWidgetQt5v2_H
#define PokemonAutomation_VideoPipeline_VideoWidgetQt5v2_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <mutex>
#include <condition_variable>
#include <QVideoFrame>
#include <QThread>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "VideoWidget.h"
//#include "CameraInfo.h"
#include "CameraImplementations.h"
#include "VideoToolsQt5.h"

class QCamera;
class QCameraImageCapture;
class QCameraViewfinder;
class QVideoProbe;

namespace PokemonAutomation{
    class LoggerQt;
    class CameraInfo;
namespace CameraQt5QCameraViewfinderSeparateThread{


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


class VideoWidget;


class CameraHolder : public QObject, public PokemonAutomation::Camera{
    Q_OBJECT
public:
    CameraHolder(
        Logger& logger,
        const CameraInfo& info, const Resolution& desired_resolution
    );
    virtual ~CameraHolder();

    virtual Resolution current_resolution() const override{ return m_current_resolution; }
    virtual std::vector<Resolution> supported_resolutions() const override{ return m_supported_resolutions; }

    virtual VideoSnapshot snapshot() override;

public slots:
    virtual void set_resolution(const Resolution& size) override;

signals:
    void stop();

private:
//    void internal_shutdown();

    //  All of these must be called under the state lock.
    QImage direct_snapshot_probe(bool flip_vertical);

    VideoSnapshot snapshot_image();
    VideoSnapshot snapshot_probe();

    bool determine_frame_orientation();

private:
    friend class VideoWidget;

    Logger& m_logger;
    QCamera* m_camera = nullptr;
    CameraScreenshotter m_screenshotter;

    std::mutex m_state_lock;
    std::condition_variable m_cv;
    bool m_stopped = false;

    size_t m_max_frame_rate;
    std::chrono::milliseconds m_frame_period;
    std::vector<Resolution> m_supported_resolutions;
    Resolution m_current_resolution;

    QVideoProbe* m_probe = nullptr;

    WallClock m_last_orientation_attempt;
    bool m_orientation_known = false;
    bool m_flip_vertical = false;

    SpinLock m_frame_lock;
    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
    VideoSnapshot m_last_snapshot;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;
};



class VideoWidget : public PokemonAutomation::VideoWidget{
    Q_OBJECT
public:
    VideoWidget(QWidget* parent, CameraHolder& camera);
    VideoWidget(
        QWidget* parent,
        Logger& logger,
        const CameraInfo& info, const Resolution& desired_resolution
    );
    virtual ~VideoWidget();

    virtual Camera& camera() override{ return *m_holder; }
    virtual void resizeEvent(QResizeEvent* event) override;

signals:
    void internal_set_resolution(const Resolution& size);

private:
//    mutable std::mutex m_lock;

    Logger& m_logger;
    std::unique_ptr<CameraHolder> m_backing;
    CameraHolder* m_holder = nullptr;
    QThread m_thread;

    QCameraViewfinder* m_camera_view = nullptr;
};






}
}
#endif
#endif
