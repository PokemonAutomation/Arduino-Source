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
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

namespace PokemonAutomation{


std::vector<CameraInfo> qt5_get_all_cameras();
std::string qt5_get_camera_name(const CameraInfo& info);




QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical);

bool determine_frame_orientation(
    Logger& logger, const ImageViewRGB32& reference, const ImageViewRGB32& image,
    bool& flip_vertical
);



class CameraScreenshotter : public QObject{
public:
    CameraScreenshotter(Logger& logger, QCamera& camera);
    virtual ~CameraScreenshotter();
    VideoSnapshot snapshot();

private:
    Logger& m_logger;
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
