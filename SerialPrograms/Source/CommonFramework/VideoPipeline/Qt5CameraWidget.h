/*  Video Widget (Qt5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Qt5VideoWidget_H
#define PokemonAutomation_Qt5VideoWidget_H

#include <condition_variable>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include "CommonFramework/Logging/Logger.h"
#include "CameraInfo.h"
#include "VideoWidget.h"

namespace PokemonAutomation{


std::vector<CameraInfo> qt5_get_all_cameras();
QString qt5_get_camera_name(const CameraInfo& info);

class Qt5VideoWidget : public VideoWidget{
public:
    Qt5VideoWidget(
        QWidget* parent,
        Logger& logger,
        const CameraInfo& info, const QSize& desired_resolution
    );
    virtual ~Qt5VideoWidget();
    virtual QSize resolution() const override;
    virtual std::vector<QSize> resolutions() const override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual QImage snapshot() override;

    virtual void resizeEvent(QResizeEvent* event) override;

private:
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

    Logger& m_logger;
    QCamera* m_camera = nullptr;
    QCameraViewfinder* m_camera_view = nullptr;
    QCameraImageCapture* m_capture = nullptr;
    std::vector<QSize> m_resolutions;

    mutable std::mutex m_lock;
    QSize m_resolution;
    std::map<int, PendingCapture> m_pending_captures;
};



}
#endif
