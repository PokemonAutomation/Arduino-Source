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

class QCamera;
class QCameraImageCapture;
class QCameraViewfinder;
class QVideoProbe;

namespace PokemonAutomation{
    class LoggerQt;
    class CameraInfo;
namespace CameraQt5{


class Qt5VideoWidget2;


class CameraHolder : public QObject{
    Q_OBJECT
public:
    CameraHolder(
        LoggerQt& logger, Qt5VideoWidget2& widget,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~CameraHolder();

    QSize current_resolution() const{ return m_current_resolution; }
    std::vector<QSize> supported_resolutions() const{ return m_supported_resolutions; }

    VideoSnapshot snapshot();

public slots:
    void set_resolution(const QSize& size);

private:
    //  All of these must be called under the state lock.
    QImage direct_snapshot_image(std::unique_lock<std::mutex>& lock);
    QImage direct_snapshot_probe(bool flip_vertical);

    VideoSnapshot snapshot_image(std::unique_lock<std::mutex>& lock);
    VideoSnapshot snapshot_probe();

    bool determine_frame_orientation(std::unique_lock<std::mutex>& lock);

private:
    friend class Qt5VideoWidget2;
    enum class CaptureStatus{
        PENDING,
        COMPLETED,
        CANCELED,
    };
    struct PendingCapture{
        CaptureStatus status = CaptureStatus::PENDING;
        QImage image;
        std::condition_variable cv;
    };

    LoggerQt& m_logger;
    Qt5VideoWidget2& m_widget;
    QCamera* m_camera = nullptr;

    std::mutex m_state_lock;
    size_t m_max_frame_rate;
    std::chrono::milliseconds m_frame_period;
    std::vector<QSize> m_supported_resolutions;
    QSize m_current_resolution;

    //  QCameraImageCapture capture method.
    QCameraImageCapture* m_capture = nullptr;
    std::map<int, PendingCapture> m_pending_captures;

    //  QVideoProbe capture method.
    QVideoProbe* m_probe = nullptr;

    WallClock m_last_orientation_attempt;
    bool m_orientation_known = false;
    bool m_flip_vertical = false;

    SpinLock m_frame_lock;
    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    uint64_t m_last_frame_seqnum = 0;

    //  Last Cached Image
    QImage m_last_image;
    WallClock m_last_image_timestamp;
    uint64_t m_last_image_seqnum = 0;
    PeriodicStatsReporterI32 m_stats_conversion;
};



class Qt5VideoWidget2 : public VideoWidget{
    Q_OBJECT
public:
    Qt5VideoWidget2(
        QWidget* parent,
        LoggerQt& logger,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~Qt5VideoWidget2();
    virtual QSize current_resolution() const override;
    virtual std::vector<QSize> supported_resolutions() const override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual VideoSnapshot snapshot() override;

    virtual void resizeEvent(QResizeEvent* event) override;

signals:
    void internal_set_resolution(const QSize& size);

private:
//    mutable std::mutex m_lock;

    LoggerQt& m_logger;
    std::unique_ptr<CameraHolder> m_holder;
    QThread m_thread;

    QCameraViewfinder* m_camera_view = nullptr;
};






}
}
#endif
#endif
