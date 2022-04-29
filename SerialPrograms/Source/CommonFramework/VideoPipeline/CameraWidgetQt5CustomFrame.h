/*  Video Widget (Custom Frame)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This is an experimental implementation where we handle the rendering
 *  ourselves in order to move it off the UI thread.
 *
 *  Long story short: It didn't work. :D
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoWidgetQt5CustomFrame_H
#define PokemonAutomation_VideoPipeline_VideoWidgetQt5CustomFrame_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <mutex>
#include <condition_variable>
#include <QThread>
#include <QCamera>
#include <QVideoProbe>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CameraImplementations.h"
#include "VideoToolsQt5.h"

namespace PokemonAutomation{
namespace CameraQt5CustomFrame{


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual QString get_camera_name(const CameraInfo& info) const override;
    virtual VideoWidget* make_video_widget(
        QWidget& parent,
        LoggerQt& logger,
        const CameraInfo& info,
        const QSize& desired_resolution
    ) const override;
};





class VideoWidget;



class CameraMode{
public:
    virtual ~CameraMode() = default;
    virtual void set_widget_dimensions(QSize size){}
    virtual VideoSnapshot snapshot() = 0;
    virtual QImage get_display_image(){ return QImage(); }
};







class CameraModeLegacy : public QObject, public CameraMode{
public:
    CameraModeLegacy(LoggerQt& logger, QCamera& camera);

    virtual VideoSnapshot snapshot() override;


private:
    std::chrono::milliseconds m_frame_period;
    CameraScreenshotter m_screenshotter;

    std::mutex m_snapshot_lock;

    VideoSnapshot m_last_snapshot;
};

class CameraModeProbeExpiration : public QObject, public CameraMode{
public:
    CameraModeProbeExpiration(LoggerQt& logger, QCamera& camera, QVideoProbe& probe);

    virtual VideoSnapshot snapshot() override;


private:
    std::chrono::milliseconds m_frame_period;
    CameraScreenshotter m_screenshotter;
    QVideoProbe& m_probe;

    std::mutex m_snapshot_lock;
    std::atomic<uint64_t> m_last_frame_seqnum;
    uint64_t m_last_image_seqnum;
    VideoSnapshot m_last_snapshot;
};

class CameraModeSelfRender : public QObject, public CameraMode{
    Q_OBJECT
public:
    CameraModeSelfRender(LoggerQt& logger, VideoWidget& widget, QVideoProbe& probe, bool flip_vertical);
    virtual ~CameraModeSelfRender();

    virtual void set_widget_dimensions(QSize size) override;
    virtual VideoSnapshot snapshot() override;
    virtual QImage get_display_image() override;

signals:
    void new_frame();

private:
    uint64_t snapshot(VideoSnapshot& image);    //  Must call under "m_cache_lock".
    void thread_loop();

private:
    LoggerQt& m_logger;
    QVideoProbe& m_probe;

    std::atomic<bool> m_stopping;
    bool m_flip_vertical;

    std::atomic<int> m_widget_width;
    std::atomic<int> m_widget_height;

    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
    VideoSnapshot m_last_image;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;

    //  Latest frame to render.
    QImage m_last_display;
    uint64_t m_last_display_seqnum = 0;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};








class CameraHolder : public QObject{
    Q_OBJECT
public:
    CameraHolder(
        LoggerQt& logger, VideoWidget& widget,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~CameraHolder();

    QSize current_resolution() const{ return m_current_resolution; }
    std::vector<QSize> supported_resolutions() const{ return m_supported_resolutions; }

    void set_view_finder(QCameraViewfinder* widget);
    void set_widget_dimensions(QSize size);

    VideoSnapshot snapshot();
    QImage get_display_image();

public slots:
    void set_resolution(const QSize& size);

signals:
    void use_view_finder(bool enabled);

private:
    LoggerQt& m_logger;

    QCamera* m_camera = nullptr;
    QVideoProbe* m_probe = nullptr;

    std::vector<QSize> m_supported_resolutions;
    QSize m_current_resolution;

    std::unique_ptr<CameraMode> m_mode;

    std::mutex m_lock;
};









class VideoWidget : public PokemonAutomation::VideoWidget{
    Q_OBJECT
public:
    VideoWidget(
        QWidget* parent,
        LoggerQt& logger,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~VideoWidget();
    virtual QSize current_resolution() const override;
    virtual std::vector<QSize> supported_resolutions() const override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual VideoSnapshot snapshot() override;

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

public slots:
    void use_view_finder(bool enabled);
    void new_frame();

signals:
    void internal_set_resolution(const QSize& size);

private:
    LoggerQt& m_logger;
    std::unique_ptr<CameraHolder> m_holder;
    QThread m_thread;

    QCameraViewfinder* m_camera_view = nullptr;
};




}
}
#endif
#endif
