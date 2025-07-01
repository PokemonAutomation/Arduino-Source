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
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "QVideoFrameCache.h"
#include "CameraImplementations.h"

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{


class CameraBackend : public PokemonAutomation::CameraBackend{
public:
    virtual std::vector<CameraInfo> get_all_cameras() const override;
    virtual std::string get_camera_name(const CameraInfo& info) const override;

    virtual std::unique_ptr<VideoSource> make_video_source(
        Logger& logger,
        const CameraInfo& info,
        Resolution resolution
    ) const override;
};








class CameraVideoSource : public QObject, public VideoSource{
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

    virtual VideoSnapshot snapshot() override;

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

private:
//    void set_video_output(QGraphicsVideoItem& item);


private:
    friend class CameraVideoDisplay;

    Logger& m_logger;
    Resolution m_resolution;

    std::mutex m_snapshot_lock;

    std::unique_ptr<QCamera> m_camera;
    std::unique_ptr<QVideoSink> m_video_sink;
    std::unique_ptr<QMediaCaptureSession> m_capture;

    std::vector<Resolution> m_resolutions;

private:
    //  Last Cached Image: All accesses must be under this lock.

    QImage m_last_image;
    WallClock m_last_image_timestamp;
    uint64_t m_last_image_seqnum = 0;

    PeriodicStatsReporterI32 m_stats_conversion;


private:
    QVideoFrameCache m_last_frame;

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
