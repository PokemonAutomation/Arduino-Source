/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CameraSelector_H
#define PokemonAutomation_CameraSelector_H

#include <deque>
#include <set>
#include <mutex>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VideoOverlayWidget.h"
#include "VideoWidget.h"
#include "VideoDisplayWidget.h"

namespace PokemonAutomation{



class CameraSelectorUI;


class CameraSelector{
    static const QString JSON_CAMERA;
    static const QString JSON_RESOLUTION;

public:
    CameraSelector();
    CameraSelector(const QJsonValue& json);

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    CameraSelectorUI* make_ui(QWidget& parent, Logger& logger, VideoDisplayWidget& holder);

private:
    friend class CameraSelectorUI;
    CameraInfo m_camera;
    QSize m_resolution;
};





class CameraSelectorUI : public QWidget, public VideoFeed{
    Q_OBJECT
public:
    CameraSelectorUI(
        QWidget& parent,
        Logger& logger,
        CameraSelector& value,
        VideoDisplayWidget& holder
    );
    ~CameraSelectorUI();

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
