/*  Camera Widget (Qt6 QQuickWidget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Use QML QQuickWidget.
 *  This is hardware accelerated.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt6_QQuickWidget_H
#define PokemonAutomation_VideoPipeline_Qt6_QQuickWidget_H

#include <QWidget>
#include <QVideoWidget>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QQuickWidget>
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
namespace CameraQt6_QQuickWidget{


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
        Resolution resolution,
        VideoFormat format,
        FramesPerSecond fps
    ) const override;
};



class CameraVideoSource : public VideoSource{
public:
    virtual ~CameraVideoSource();
    CameraVideoSource(
        Logger& logger,
        const CameraInfo& info,
        Resolution desired_resolution,
        VideoFormat desired_format,
        FramesPerSecond desired_fps
    );

    virtual Resolution current_resolution() const override{
        return m_resolution;
    }
    virtual VideoFormat current_format() const override{
        return m_format;
    }
    virtual FramesPerSecond current_fps() const override{
        return m_fps;
    }
    virtual const VideoFormatSet& supported_formats() const override{
        return m_formats;
    }

    virtual VideoSnapshot snapshot_latest_blocking() override{
        return m_snapshot_manager.snapshot_latest_blocking();
    }
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override{
        return m_snapshot_manager.snapshot_recent_nonblocking(min_time);
    }

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

private:
    void init(
        const CameraInfo& info,
        Resolution desired_resolution,
        VideoFormat desired_format,
        FramesPerSecond desired_fps
    );
//    void set_video_output(QGraphicsVideoItem& item);


private:
    friend class CameraVideoDisplay;

    std::unique_ptr<QObject> m_metaobject;

    Logger& m_logger;
    Resolution m_resolution;
    VideoFormat m_format;
    FramesPerSecond m_fps;

    std::unique_ptr<QCameraThread> m_camera;
    std::unique_ptr<QVideoSink> m_video_sink;
    std::unique_ptr<QMediaCaptureSession> m_capture;

    VideoFormatSet m_formats;


private:
    QVideoFrameCache m_last_frame;
    SnapshotManager m_snapshot_manager;
};



class CameraVideoDisplay : public QQuickWidget{
public:
    virtual ~CameraVideoDisplay();
    CameraVideoDisplay(QWidget* parent);

private:
    LifetimeSanitizer m_sanitizer;
};
















}
}
#endif
