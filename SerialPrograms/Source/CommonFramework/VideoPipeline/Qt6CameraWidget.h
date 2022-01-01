/*  Video Widget (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Qt6VideoWidget_H
#define PokemonAutomation_Qt6VideoWidget_H

#include <mutex>
#include "CommonFramework/Logging/Logger.h"
#include "CameraInfo.h"
#include "VideoWidget.h"

#include <QMediaDevices>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QCameraDevice>
#include <QVideoWidget>
#include <QVideoSink>
#include <QVideoFrame>
#include <QCamera>

namespace PokemonAutomation{


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
    virtual QImage snapshot() override;

    virtual void resizeEvent(QResizeEvent* event) override;
private:
    void paintEvent(QPaintEvent*) override;

    Logger& m_logger;
    std::vector<QSize> m_resolutions;

    QCameraDevice m_info;
    QCamera* m_camera = nullptr;
    QImageCapture * m_capture = nullptr;
    QMediaCaptureSession m_captureSession;
    QVideoSink* m_videoSink = nullptr;
    QVideoFrame m_videoFrame;
    std::vector<QCameraFormat> m_formats;

    mutable std::mutex m_lock;
};


}
#endif
