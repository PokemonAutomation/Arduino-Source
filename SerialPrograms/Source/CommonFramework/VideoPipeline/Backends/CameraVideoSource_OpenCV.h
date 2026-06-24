/*  Camera Video Source (OpenCV / DirectShow)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A VideoSource implementation backed by OpenCV's VideoCapture with
 *      DirectShow (CAP_DSHOW) on Windows.  This is used for capture cards
 *      that only expose a DirectShow driver and are not visible to Qt6's
 *      WMF-based QMediaDevices.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraVideoSource_OpenCV_H
#define PokemonAutomation_VideoPipeline_CameraVideoSource_OpenCV_H

#ifdef _WIN32

#include <atomic>
#include <memory>
#include <thread>
#include <QWidget>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"

namespace PokemonAutomation{


class CameraVideoSource_OpenCV : public VideoSource{
public:
    ~CameraVideoSource_OpenCV();
    CameraVideoSource_OpenCV(
        Logger& logger,
        int device_index,
        const std::string& device_name,
        Resolution desired_resolution
    );

    virtual Resolution current_resolution() const override{
        return m_resolution;
    }
    virtual const std::vector<Resolution>& supported_resolutions() const override{
        return m_resolutions;
    }

    virtual VideoSnapshot snapshot_latest_blocking() override;
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override;

    virtual QWidget* make_display_QtWidget(QWidget* parent) override;

    //  Shared flag checked by the display widget to detect source destruction.
    std::shared_ptr<std::atomic<bool>> alive_flag() const{ return m_alive; }

    //  Public wrapper so the display widget can report rendered frames.
    void notify_rendered_frame(WallClock timestamp){ report_rendered_frame(timestamp); }

private:
    void capture_thread_body(int device_index, Resolution desired_resolution);

private:
    Logger& m_logger;

    std::shared_ptr<std::atomic<bool>> m_alive;
    std::atomic<bool> m_stopping{false};
    std::thread m_capture_thread;

    mutable SpinLockMRSW m_frame_lock;
    VideoSnapshot m_latest_snapshot;

    Resolution m_resolution;
    std::vector<Resolution> m_resolutions;
};


}

#endif // _WIN32
#endif
