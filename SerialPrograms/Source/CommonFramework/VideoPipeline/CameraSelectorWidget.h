/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CameraSelectorWidget_H
#define PokemonAutomation_CameraSelectorWidget_H

#include <vector>
#include <atomic>
#include <mutex>
#include <QWidget>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CameraSelector.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{


class CameraSelectorWidget : public QWidget, public VideoFeed{
    Q_OBJECT
public:
    CameraSelectorWidget(
        QWidget& parent,
        Logger& logger,
        CameraSelector& value,
        VideoDisplayWidget& holder
    );
    ~CameraSelectorWidget();

    void set_camera_enabled(bool enabled);
    void set_resolution_enabled(bool enabled);
    void set_snapshots_allowed(bool enabled);
    void set_overlay_enabled(bool enabled);

    void reset_video();
    virtual void async_reset_video() override;

    //  Cannot be called from UI thread.
    virtual QImage snapshot() override;

signals:
    void internal_async_reset_video();

private:
    void refresh();
    static QString aspect_ratio(const QSize& size);

private:
    Logger& m_logger;
    CameraSelector& m_value;

    VideoDisplayWidget& m_display;
    std::vector<QSize> m_resolutions;

    QComboBox* m_camera_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    std::vector<CameraInfo> m_cameras;

    std::atomic<bool> m_snapshots_allowed;
    std::mutex m_camera_lock;
};


}
#endif
