/*  Camera Video Source (OpenCV / DirectShow)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include <QVideoFrame>
#include <QPainter>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Logging/Logger.h"
#include "VideoFrameQt.h"
#include "CameraVideoSource_OpenCV.h"

namespace PokemonAutomation{


CameraVideoSource_OpenCV::~CameraVideoSource_OpenCV(){
    //  Signal the alive flag FIRST so the display widget stops accessing us.
    m_alive->store(false, std::memory_order_release);
    m_stopping.store(true, std::memory_order_release);
    if (m_capture_thread.joinable()){
        m_capture_thread.join();
    }
}

CameraVideoSource_OpenCV::CameraVideoSource_OpenCV(
    Logger& logger,
    int device_index,
    const std::string& device_name,
    Resolution desired_resolution
)
    : VideoSource(logger, true)
    , m_logger(logger)
    , m_alive(std::make_shared<std::atomic<bool>>(true))
{
    m_logger.log(
        "Starting Camera: Backend = OpenCV/DirectShow, device=" +
        std::to_string(device_index) + " (" + device_name + ")"
    );

    m_resolutions = {
        {1280, 720},
        {1920, 1080},
    };
    m_resolution = desired_resolution ? desired_resolution : Resolution(1920, 1080);

    m_capture_thread = std::thread(
        &CameraVideoSource_OpenCV::capture_thread_body, this,
        device_index, m_resolution
    );
}


void CameraVideoSource_OpenCV::capture_thread_body(
    int device_index,
    Resolution desired_resolution
){
    cv::VideoCapture cap;
    if (!cap.open(device_index, cv::CAP_DSHOW)){
        m_logger.log("OpenCV: Failed to open DirectShow device " + std::to_string(device_index), COLOR_RED);
        return;
    }

    if (desired_resolution){
        cap.set(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(desired_resolution.width));
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(desired_resolution.height));
    }

    //  Read actual resolution.
    int actual_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int actual_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_resolution = Resolution(actual_width, actual_height);

    m_logger.log(
        "OpenCV: Opened DirectShow device " + std::to_string(device_index) +
        " at " + std::to_string(actual_width) + "x" + std::to_string(actual_height)
    );

    //  Update supported resolutions to include the actual resolution.
    {
        bool found = false;
        for (const auto& r : m_resolutions){
            if (r == m_resolution){
                found = true;
                break;
            }
        }
        if (!found){
            m_resolutions.push_back(m_resolution);
        }
    }

    cv::Mat bgr_frame;
    while (!m_stopping.load(std::memory_order_acquire)){
        if (!cap.read(bgr_frame)){
            //  Brief sleep on read failure to avoid busy-spin.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (bgr_frame.empty()){
            continue;
        }

        WallClock now = current_time();

        //  Convert BGR -> BGRA (ImageRGB32 uses QImage::Format_ARGB32).
        //  On little-endian, BGRA bytes [B,G,R,A] map to uint32 0xAARRGGBB
        //  which matches Qt's Format_ARGB32.
        cv::Mat bgra_frame;
        cv::cvtColor(bgr_frame, bgra_frame, cv::COLOR_BGR2BGRA);

        //  Create ImageRGB32 from the cv::Mat data.
        ImageRGB32 image(bgra_frame.cols, bgra_frame.rows);
        const size_t row_bytes = static_cast<size_t>(bgra_frame.cols) * 4;
        for (int y = 0; y < bgra_frame.rows; y++){
            memcpy(
                reinterpret_cast<uint8_t*>(image.data()) + y * image.bytes_per_row(),
                bgra_frame.ptr(y),
                row_bytes
            );
        }

        VideoSnapshot snapshot(std::move(image), now);

        {
            WriteSpinLock lg(m_frame_lock, "CameraVideoSource_OpenCV::capture_thread_body()");
            m_latest_snapshot = std::move(snapshot);
        }

        //  Report the new frame for FPS tracking and listener notifications.
        report_source_frame(std::make_shared<VideoFrame>(now, QVideoFrame()));
    }

    cap.release();
}


VideoSnapshot CameraVideoSource_OpenCV::snapshot_latest_blocking(){
    ReadSpinLock lg(m_frame_lock);
    return m_latest_snapshot;
}
VideoSnapshot CameraVideoSource_OpenCV::snapshot_recent_nonblocking(WallClock min_time){
    ReadSpinLock lg(m_frame_lock);
    return m_latest_snapshot;
}


class VideoWidget_OpenCV : public QWidget{
public:
    VideoWidget_OpenCV(QWidget* parent, CameraVideoSource_OpenCV& source)
        : QWidget(parent)
        , m_source(source)
        , m_alive(source.alive_flag())
    {
        //  Prevent background erase to avoid flicker.
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);

        //  Start a refresh timer for display.
        m_timer_id = startTimer(33);  // ~30 fps
    }
    ~VideoWidget_OpenCV(){
        killTimer(m_timer_id);
    }

protected:
    void timerEvent(QTimerEvent*) override{
        //  Stop if the source has been destroyed.
        if (!m_alive->load(std::memory_order_acquire)){
            killTimer(m_timer_id);
            m_timer_id = 0;
            return;
        }
        update();
    }
    void paintEvent(QPaintEvent*) override{
        QPainter painter(this);

        //  If the source has been destroyed, just fill black.
        if (!m_alive->load(std::memory_order_acquire)){
            painter.fillRect(rect(), Qt::black);
            return;
        }

        VideoSnapshot snapshot = m_source.snapshot_latest_blocking();
        if (!snapshot){
            painter.fillRect(rect(), Qt::black);
            return;
        }

        QImage qimage = snapshot.frame->to_QImage_ref();
        if (qimage.isNull()){
            painter.fillRect(rect(), Qt::black);
            return;
        }

        painter.drawImage(
            rect(),
            qimage,
            QRect(0, 0, qimage.width(), qimage.height())
        );

        //  Report rendered frame for display FPS tracking.
        m_source.notify_rendered_frame(current_time());
    }

private:
    CameraVideoSource_OpenCV& m_source;
    std::shared_ptr<std::atomic<bool>> m_alive;
    int m_timer_id;
};

QWidget* CameraVideoSource_OpenCV::make_display_QtWidget(QWidget* parent){
    return new VideoWidget_OpenCV(parent, *this);
}


}

#endif // _WIN32
