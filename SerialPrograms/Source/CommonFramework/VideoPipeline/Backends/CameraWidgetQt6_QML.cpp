#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include <QCamera>
#include <QVideoSink>
#include <QQuickItem>
#include <QQmlEngine>
#include <QTemporaryFile>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "CameraWidgetQt6.h"
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

    QCameraFormat format = CameraQt6QVideoSink::build_format_set(
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

    CameraQt6QVideoSink::get_format(format, m_resolution, m_format, m_fps);
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

    // Auto-recovery: If the capture card glitches and the pipeline breaks, try to automatically restart it.
    m_metaobject->connect(&m_camera->camera(), &QCamera::errorOccurred, 
                          &m_camera->camera(), [this](QCamera::Error, const QString& errorString){
        m_logger.log("Camera error detected: " + errorString.toStdString() + ". Attempting auto-restart...", COLOR_RED);
        QMetaObject::invokeMethod(&m_camera->camera(), "start", Qt::QueuedConnection);
    });
}

QWidget* CameraVideoSource::make_display_QtWidget(QWidget* parent){
    CameraVideoDisplay* display = new CameraVideoDisplay(parent, *this);
    
    QObject* root = display->rootObject();
    if (root){
        QObject* vo = root->findChild<QObject*>("videoOutput");
        if (vo){
            QVideoSink* qml_sink = vo->property("videoSink").value<QVideoSink*>();
            if (qml_sink){
                m_metaobject->disconnect();
                m_metaobject->connect(
                    m_video_sink.get(), &QVideoSink::videoFrameChanged,
                    qml_sink, [this, qml_sink](const QVideoFrame& frame){
                        WallClock now = current_time();
                        if (!m_last_frame.push_frame(frame, now)){
                            return;
                        }
                        
                        report_source_frame(std::make_shared<VideoFrame>(now, frame));
                        
                        QMetaObject::invokeMethod(qml_sink, [qml_sink, frame]() {
                            qml_sink->setVideoFrame(frame);
                        }, Qt::QueuedConnection);
                        
                        report_rendered_frame(now);
                    }
                );
            }
        }
    }
    
    return display;
}

CameraVideoDisplay::~CameraVideoDisplay(){
    m_source.remove_source_frame_listener(*this);
}
CameraVideoDisplay::CameraVideoDisplay(QWidget* parent, CameraVideoSource& source)
    : QQuickWidget(parent)
    , m_source(source)
    , m_sanitizer("CameraVideoDisplay")
{
    this->setMinimumSize(80, 45);
    this->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
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
    
    source.add_source_frame_listener(*this);
}

void CameraVideoDisplay::on_frame(std::shared_ptr<const VideoFrame> frame){
    // Not needed, QML handles the rendering directly via SceneGraph
}

}
}
#endif
