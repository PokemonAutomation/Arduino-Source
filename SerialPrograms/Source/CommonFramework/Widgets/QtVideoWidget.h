/*  Video Widget (Qt)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_QtVideoWidget_H
#define PokemonAutomation_QtVideoWidget_H

#include <condition_variable>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include "CommonFramework/Tools/Logger.h"
#include "VideoWidget.h"

namespace PokemonAutomation{


class QtVideoWidget : public VideoWidget{
public:
    QtVideoWidget(
        QWidget& parent, Logger& logger,
        const QCameraInfo& info, const QSize& suggested_resolution
    );
    virtual ~QtVideoWidget();
    virtual QSize resolution() override;
    virtual std::vector<QSize> resolutions() override;
    virtual void set_resolution(const QSize& size) override;

    //  Cannot call from UI thread or it will deadlock.
    virtual QImage snapshot() override;


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

    std::mutex m_lock;
    QSize m_resolution;
    std::map<int, PendingCapture> m_pending_captures;
};



}
#endif
