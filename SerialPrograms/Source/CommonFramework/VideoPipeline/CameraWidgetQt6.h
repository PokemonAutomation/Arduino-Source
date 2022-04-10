/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt6VideoWidget_H
#define PokemonAutomation_VideoPipeline_Qt6VideoWidget_H

#include <mutex>
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CameraInfo.h"
#include "VideoWidget.h"

class QCamera;
class QVideoSink;

namespace PokemonAutomation{
namespace CameraQt6{


std::vector<CameraInfo> qt6_get_all_cameras();
QString qt6_get_camera_name(const CameraInfo& info);

class Qt6VideoWidget : public VideoWidget{
public:
    Qt6VideoWidget(
        QWidget* parent,
        Logger& logger,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~Qt6VideoWidget();
    virtual QSize resolution() const override;
    virtual std::vector<QSize> resolutions() const override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual QImage snapshot(WallClock* timestamp) override;

    virtual void resizeEvent(QResizeEvent* event) override;
private:
    void paintEvent(QPaintEvent*) override;

    Logger& m_logger;
    std::vector<QSize> m_resolutions;

    QCameraDevice m_info;
    QCamera* m_camera = nullptr;
    QMediaCaptureSession m_captureSession;
    QVideoSink* m_videoSink = nullptr;
    QVideoFrame m_videoFrame;
    WallClock m_videoTimestamp;
    std::vector<QCameraFormat> m_formats;

    mutable std::mutex m_lock;
    std::mutex m_image_lock;
    SpinLock m_frame_lock;
    std::atomic<uint64_t> m_seqnum_frame;
//    uint64_t m_seqnum_frame;
    uint64_t m_seqnum_image = 0;
    QImage m_cached_frame;
    WallClock m_cached_timestamp;
};


}
}
#endif
