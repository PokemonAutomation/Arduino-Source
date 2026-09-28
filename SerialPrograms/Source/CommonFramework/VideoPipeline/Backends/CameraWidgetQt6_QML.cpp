/*  Camera Widget (Qt6 QML VideoOutput)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QCamera>
#include <QVideoSink>
#include <QQuickItem>
#include <QQmlEngine>
#include <QTemporaryFile>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "QFormatAggregator.h"
#include "CameraWidgetQt6_QML.h"
#include "MediaServicesQt6.h"
#include "VideoFrameQt.h"

namespace PokemonAutomation{
namespace CameraQt6QML{

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
    global_logger_tagged().log("Error: No such camera for CameraInfo: " + info.device_name(), COLOR_RED);
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

    run_on_main_thread_and_wait([&]{
        m_metaobject.reset();
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
    m_logger.log("Starting Camera: Backend = CameraQt6QML");

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
    m_logger.log("Camera: " + device->description().toStdString());

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

    //  Restart the camera if the capture pipeline breaks.
    m_metaobject->connect(
        &m_camera->camera(), &QCamera::errorOccurred,
        &m_camera->camera(), [this](QCamera::Error, const QString& errorString){
            m_logger.log("Camera error detected: " + errorString.toStdString() + ". Attempting auto-restart...", COLOR_RED);
            QMetaObject::invokeMethod(&m_camera->camera(), "start", Qt::QueuedConnection);
        }
    );

    //  Context is the QCamera: runs on the camera thread and outlives every display.
    m_metaobject->connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
        &m_camera->camera(), [this](const QVideoFrame& frame){
            WallClock copy_start = current_time();
            QVideoFrame owned = deep_copy_frame(frame);
            WallClock copy_end = current_time();
            report_video_path_stats(
                m_logger, "QML", frame, owned.isValid(), copy_start, copy_end
            );
            if (!owned.isValid()){
                return;
            }

            WallClock now = current_time();
            if (!m_last_frame.push_frame(owned, now)){
                return;
            }
            report_source_frame(std::make_shared<VideoFrame>(now, owned));
        }
    );
}

QWidget* CameraVideoSource::make_display_QtWidget(QWidget* parent){
    CameraVideoDisplay* display = new CameraVideoDisplay(parent);

    //  Null if init() found no camera or usable format.
    if (m_camera == nullptr || m_video_sink == nullptr){
        m_logger.log("make_display_QtWidget(): camera was not initialized", COLOR_RED);
        return display;
    }

    QObject* root = display->rootObject();
    if (root == nullptr){
        m_logger.log("make_display_QtWidget(): display->rootObject() == nullptr", COLOR_RED);
        return display;
    }

    QObject* vo = root->findChild<QObject*>("videoOutput");
    if (vo == nullptr){
        m_logger.log("make_display_QtWidget(): videoOutput == nullptr", COLOR_RED);
        return display;
    }

    QVideoSink* qml_sink = vo->property("videoSink").value<QVideoSink*>();
    if (qml_sink == nullptr){
        m_logger.log("make_display_QtWidget(): videoSink == nullptr", COLOR_RED);
        return display;
    }

    QObject::disconnect(m_display_connection);

    //  qml_sink must be the context: it is destroyed whenever the display is
    //  rebuilt, which drops this connection, and it queues the call onto the
    //  GUI thread.
    m_display_connection = QObject::connect(
        m_video_sink.get(), &QVideoSink::videoFrameChanged,
        qml_sink, [this, qml_sink](const QVideoFrame& frame){
            qml_sink->setVideoFrame(frame);
            report_rendered_frame(current_time());
        }
    );

    return display;
}

CameraVideoDisplay::~CameraVideoDisplay(){
}
CameraVideoDisplay::CameraVideoDisplay(QWidget* parent)
    : QQuickWidget(parent)
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    this->setResizeMode(QQuickWidget::SizeRootObjectToView);
    this->setFocusPolicy(Qt::NoFocus);
    
    QString qml = QStringLiteral(
        "import QtQuick\n"
        "import QtMultimedia\n"
        "Rectangle {\n"
        "    color: \"black\"\n"
        "    VideoOutput {\n"
        "        id: videoOutput\n"
        "        anchors.fill: parent\n"
        "        objectName: \"videoOutput\"\n"
        "        fillMode: VideoOutput.PreserveAspectFit\n"
        "    }\n"
        "}\n"
    );
    
    QTemporaryFile file;
    if (file.open()){
        file.write(qml.toUtf8());
        file.close();
        this->setSource(QUrl::fromLocalFile(file.fileName()));
    }
}

}
}
#endif
