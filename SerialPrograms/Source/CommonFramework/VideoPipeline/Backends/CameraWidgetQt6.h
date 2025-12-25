/*  Camera Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt6VideoWidget_H
#define PokemonAutomation_VideoPipeline_Qt6VideoWidget_H

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <mutex>
#include <QWidget>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "QCameraThread.h"
#include "QVideoFrameCache.h"
#include "SnapshotManager.h"
#include "CameraImplementations.h"

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    // Get all cameras' info.
    // Note: to avoid freezing the UI while launching the application, the camera backend when
    // constructed, use a separate thread to query and load camera info. If you call
    // `get_all_cameras()` immidiately after the backend is constructed, it may not give you
    // all cameras' info.
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<VideoSource> make_video_source(
        Logger& logger,
        const CameraInfo& info,
        Resolution resolution
    ) const override;
};








class CameraVideoSource : public VideoSource{
public:
    virtual ~CameraVideoSource();
    CameraVideoSource(
        Logger& logger,
        const CameraInfo& info,
        Resolution desired_resolution
    );

    virtual Resolution current_resolution() const override{
        return m_resolution;
    }
    virtual const std::vector<Resolution>& supported_resolutions() const override{
        return m_resolutions;
    }

    virtual VideoSnapshot snapshot_latest_blocking() override{
        return m_snapshot_manager.snapshot_latest_blocking();
    }
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override{
        return m_snapshot_manager.snapshot_recent_nonblocking(min_time);
    }

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

private:
    void init(const CameraInfo& info, Resolution desired_resolution);
//    void set_video_output(QGraphicsVideoItem& item);


private:
    friend class CameraVideoDisplay;

    std::unique_ptr<QObject> m_metaobject;

    Logger& m_logger;
    Resolution m_resolution;

    std::mutex m_snapshot_lock;

    std::unique_ptr<QCameraThread> m_camera;
    std::unique_ptr<QVideoSink> m_video_sink;
    std::unique_ptr<QMediaCaptureSession> m_capture;

    std::vector<Resolution> m_resolutions;


private:
    QVideoFrameCache m_last_frame;
    SnapshotManager m_snapshot_manager;
};


class CameraVideoDisplay : public QWidget, private VideoFrameListener{
public:
    ~CameraVideoDisplay();
    CameraVideoDisplay(QWidget* parent, CameraVideoSource& source);

private:
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    CameraVideoSource& m_source;
    std::shared_ptr<const VideoFrame> m_last_frame;

    LifetimeSanitizer m_sanitizer;
};
















}
}
#endif
#endif
