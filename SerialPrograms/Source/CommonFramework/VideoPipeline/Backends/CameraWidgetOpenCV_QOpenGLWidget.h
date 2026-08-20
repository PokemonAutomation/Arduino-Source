/*  Camera Widget (OpenCV V4L2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CameraWidgetOpenCV_QOpenGLWidget_H
#define PokemonAutomation_CameraWidgetOpenCV_QOpenGLWidget_H

#include <QtGlobal>
#if defined(__linux__) || defined(__APPLE__)

#include <QWidget>
#include <QOpenGLWidget>
#include <thread>
#include <atomic>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CameraImplementations.h"
#include "QVideoFrameCache.h"
#include "SnapshotManager.h"

// Forward declaration for OpenCV
namespace cv {
    class VideoCapture;
    class Mat;
}

namespace PokemonAutomation{
namespace CameraOpenCV_QOpenGLWidget{

class CameraBackend : public PokemonAutomation::CameraBackend{
public:
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
    void thread_body();

private:
    friend class CameraVideoDisplay;

    Logger& m_logger;
    Resolution m_resolution;
    VideoFormat m_format;
    FramesPerSecond m_fps;

    VideoFormatSet m_formats;

    int m_device_index = 0;
    std::string m_camera_description;
    std::unique_ptr<cv::VideoCapture> m_cap;
    std::thread m_thread;
    std::atomic<bool> m_stop;

    QVideoFrameCache m_last_frame;
    SnapshotManager m_snapshot_manager;
};

class CameraVideoDisplay : public QOpenGLWidget, private VideoFrameListener{
public:
    ~CameraVideoDisplay();
    CameraVideoDisplay(QWidget* parent, CameraVideoSource& source);

private:
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    CameraVideoSource& m_source;
    std::shared_ptr<const VideoFrame> m_last_frame;

    std::atomic<int> m_width;
    std::atomic<int> m_height;

    LifetimeSanitizer m_sanitizer;
};

}
}
#endif
#endif
