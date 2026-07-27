/*  Camera Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QCamera>
#include <QPainter>
#include <QMediaDevices>
#include <QVideoSink>
//#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Time.h"
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoFrameQt.h"
#include "MediaServicesQt6.h"
#include "CameraWidgetQt6.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace CameraQt6QVideoSink{



void get_format(
    const QCameraFormat& qformat,
    Resolution& resolution,
    VideoFormat& format,
    FramesPerSecond& fps
){
    resolution.width = qformat.resolution().width();
    resolution.height = qformat.resolution().height();
    format = QVideoFrameFormat_to_VideoFormat(qformat.pixelFormat());
    float fps_f = qformat.maxFrameRate();
    fps_f = std::max<float>(fps_f, 0);
    fps = (size_t)(fps_f + 0.5);
}


struct FormatAggregator{
    std::string m_format_dump_str;
    std::map<
        Resolution,
        std::map<
            VideoFormat,
            std::map<size_t, QCameraFormat, std::greater<size_t>>
        >
    > m_formats;

    void add_format(QCameraFormat qformat){
        Resolution resolution;
        VideoFormat oformat;
        FramesPerSecond fps;
        get_format(qformat, resolution, oformat, fps);

        m_format_dump_str += "\n    ";
        m_format_dump_str += std::to_string(resolution.width) + "x" + std::to_string(resolution.height);
        m_format_dump_str += ": Format = " + std::to_string((int)qformat.pixelFormat());
        m_format_dump_str += ", FPS Range = [" + std::to_string(qformat.minFrameRate()) + "," + std::to_string(qformat.maxFrameRate()) + "]";

        auto& resolution_entry = m_formats[resolution];
        auto iter0 = resolution_entry.find(oformat);

        //  Format doesn't exist. Add it.
        if (iter0 == resolution_entry.end()){
            auto& entry = resolution_entry[oformat];
            entry[fps] = std::move(qformat);
            return;
        }

        //  Format already exists.

        auto& oformat_node = iter0->second;

        auto iter1 = oformat_node.find(fps);
        if (iter1 == oformat_node.end()){
            oformat_node[fps] = std::move(qformat);
            return;
        }

        if (iter1->second.minFrameRate() < qformat.minFrameRate()){
            iter1->second = std::move(qformat);
        }
    }
};



QCameraFormat build_format_set(
    Logger& logger,
    VideoFormatSet& format_set,
    const QCameraDevice& device,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
){
    QList<QCameraFormat> formats = device.videoFormats();
    if (formats.empty()){
        logger.log("No usable resolutions: " + device.description().toStdString(), COLOR_RED);
        return QCameraFormat();
    }

    FormatAggregator aggregator;
    for (QCameraFormat& format : formats){
        aggregator.add_format(std::move(format));
    }

    if (GlobalSettings::instance().DUMP_VIDEO_FORMATS){
        logger.log("Video Formats:" + aggregator.m_format_dump_str);
    }

//    cout << "Chosen: " << resolution_map[Resolution(3840, 2160)]->maxFrameRate() << endl;

    //  Set a default.
    const QCameraFormat* current_qformat = &aggregator.m_formats.begin()->second.begin()->second.begin()->second;

    format_set.clear();
    for (const auto& res : aggregator.m_formats){
        //  Resolution matches. Pick the first one as the default.
        if (res.first == desired_resolution){
            current_qformat = &res.second.begin()->second.begin()->second;
        }

        for (const auto& format : res.second){
            //  Format matches. Pick the first one as the default.
            if (res.first == desired_resolution && format.first == desired_format){
                current_qformat = &format.second.begin()->second;
            }

            for (const auto& fps : format.second){
                //  FPS matches. Pick it.
                if (res.first == desired_resolution &&
                    format.first == desired_format &&
                    fps.first == desired_fps
                ){
                    current_qformat = &fps.second;
                }

                format_set[res.first][format.first].insert(fps.first);
            }
        }
    }

    return *current_qformat;
}






std::vector<CameraInfo> CameraBackend::get_all_cameras() const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    std::vector<CameraInfo> ret;
    for (const auto& info : cameras){
        ret.emplace_back(info.id().toStdString());
    }
    return ret;
}
std::string CameraBackend::get_camera_name(const CameraInfo& info) const{
#if 1
    const auto cameras = GlobalMediaServices::instance().get_all_cameras();
#else
    const auto cameras = QMediaDevices::videoInputs();
#endif
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            return camera.description().toStdString();
        }
    }
    std::cout << "Error: no such camera for CameraInfo: " << info.device_name() << std::endl;
    return "";
}
std::unique_ptr<VideoSource> CameraBackend::make_video_source(
    Logger& logger,
    const CameraInfo& info,
    Resolution resolution,
    VideoFormat format,
    FramesPerSecond fps
) const{
    return std::make_unique<CameraVideoSource>(logger, info, resolution, format, fps);
}






CameraVideoSource::~CameraVideoSource(){
    if (!m_capture){
        return;
    }
    try{
        m_logger.log("Stopping Camera...");
    }catch (...){}

//    cout << "Stopping Camera..." << endl;

    run_on_main_thread_and_wait([&]{
        m_metaobject.reset();
//        m_camera->stop();
        m_capture.reset();
        m_video_sink.reset();
        m_camera.reset();
    });
}
CameraVideoSource::CameraVideoSource(
    Logger& logger,
    const CameraInfo& info,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
)
    : VideoSource(logger, true)
    , m_logger(logger)
    , m_last_frame(logger)
    , m_snapshot_manager(logger, m_last_frame)
{
    if (!info){
        return;
    }
    m_logger.log("Starting Camera: Backend = CameraQt6QVideoSink");

    run_on_main_thread_and_wait([&]{
        init(info, desired_resolution, desired_format, desired_fps);
    });
}
void CameraVideoSource::init(
    const CameraInfo& info,
    Resolution desired_resolution,
    VideoFormat desired_format,
    FramesPerSecond desired_fps
){
    m_metaobject.reset(new QObject());

    auto cameras = QMediaDevices::videoInputs();
    const QCameraDevice* device = nullptr;
    for (const auto& camera : cameras){
        if (camera.id().toStdString() == info.device_name()){
            device = &camera;
            break;
        }
    }
    if (device == nullptr){
        m_logger.log("Camera not found: " + info.device_name(), COLOR_RED);
        return;
    }

    QCameraFormat format = build_format_set(
        m_logger,
        m_formats,
        *device,
        desired_resolution,
        desired_format,
        desired_fps
    );
    if (format.isNull()){
        return;
    }

    get_format(format, m_resolution, m_format, m_fps);
    m_logger.log(
        "Resolution: " + m_resolution.to_string() +
        ", Format: " + VideoFormat_database().find(m_format)->display +
        ", FPS: " + std::to_string(m_fps)
    );

    m_camera.reset(new QCameraThread(m_logger, *device, format));
    m_video_sink.reset(new QVideoSink());
    m_capture.reset(new QMediaCaptureSession());
    m_capture->setCamera(&m_camera->camera());
    m_capture->setVideoSink(m_video_sink.get());

#if 0
    connect(m_camera.get(), &QCamera::errorOccurred, this, [&](){
        if (m_camera->error() != QCamera::NoError){
            m_logger.log("QCamera error: " + m_camera->errorString().toStdString());
        }
    });
#endif
    m_metaobject->connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
        &m_camera->camera(), [&](const QVideoFrame& frame){
            //  This runs on the QCamera's thread. So it is off the critical path.

            WallClock now = current_time();
            if (!m_last_frame.push_frame(frame, now)){
                return;
            }
            report_source_frame(std::make_shared<VideoFrame>(now, frame));
        }
    );
}


QWidget* CameraVideoSource::make_display_QtWidget(QWidget* parent){
    return new CameraVideoDisplay(parent, *this);
}





CameraVideoDisplay::~CameraVideoDisplay(){
    m_source.remove_source_frame_listener(*this);
}
CameraVideoDisplay::CameraVideoDisplay(QWidget* parent, CameraVideoSource& source)
    : QWidget(parent)
    , m_source(source)
{
    source.add_source_frame_listener(*this);
}
void CameraVideoDisplay::on_frame(std::shared_ptr<const VideoFrame> frame){
    m_last_frame = frame;
    this->update();
}
void CameraVideoDisplay::paintEvent(QPaintEvent* event){
    QWidget::paintEvent(event);

    if (!m_last_frame){
        return;
    }

    QVideoFrame frame = m_last_frame->frame;
    if (!frame.isValid()){
        return;
    }

    QRect rect(0, 0, this->width(), this->height());
    QVideoFrame::PaintOptions options;
    QPainter painter(this);

    frame.paint(&painter, rect, options);
    m_source.report_rendered_frame(current_time());
}



















}
}
#endif
