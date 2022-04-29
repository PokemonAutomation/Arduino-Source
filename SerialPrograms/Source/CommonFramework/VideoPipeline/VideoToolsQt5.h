/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoToolsQt5_H
#define PokemonAutomation_VideoPipeline_VideoToolsQt5_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <QImage>
#include <QCamera>
#include <QVideoFrame>
#include <QCameraImageCapture>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CameraInfo.h"
#include "VideoFeed.h"

namespace PokemonAutomation{


std::vector<CameraInfo> qt5_get_all_cameras();
QString qt5_get_camera_name(const CameraInfo& info);




QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical);

bool determine_frame_orientation(
    Logger& logger, const QImage& reference, const QImage& image,
    bool& flip_vertical
);



class CameraScreenshotter : public QObject{
public:
    CameraScreenshotter(LoggerQt& logger, QCamera& camera);
    virtual ~CameraScreenshotter();
    VideoSnapshot snapshot();

private:
    LoggerQt& m_logger;
    QCamera& m_camera;
    PeriodicStatsReporterI32 m_stats_conversion;

    std::mutex m_lock;
    std::condition_variable m_cv;

    QCameraImageCapture* m_capture = nullptr;

    bool m_pending_screenshot;
    QImage m_screenshot;
};






}
#endif
