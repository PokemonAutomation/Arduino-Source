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
#include <QCameraInfo>
#include <QComboBox>
#include <QPushButton>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VideoOverlayWidget.h"
#include "VideoWidget.h"

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

    CameraSelectorUI* make_ui(QWidget& parent, Logger& logger, QWidget& holder);

private:
    friend class CameraSelectorUI;
    QCameraInfo m_camera;
    QSize m_resolution;
};





class CameraSelectorUI : public QWidget, public VideoFeed, public VideoOverlay{
public:
    CameraSelectorUI(
        QWidget& parent,
        Logger& logger,
        CameraSelector& value,
        QWidget& holder
    );
    ~CameraSelectorUI();

    VideoOverlayWidget& overlay(){ return *m_overlay; }

    void set_camera_enabled(bool enabled);
    void set_resolution_enabled(bool enabled);
    void set_snapshots_allowed(bool enabled);
    void set_overlay_enabled(bool enabled);

    void update_size();
    void reset_video();

    virtual QImage snapshot() override;
    virtual void add_box(const ImageFloatBox& box, QColor color) override;
    virtual void remove_box(const ImageFloatBox& box) override;
//    virtual void test_draw() override;

private:
    void refresh();
    static QString aspect_ratio(const QSize& size);

private:
    Logger& m_logger;
    CameraSelector& m_value;
    QWidget& m_holder;

    QComboBox* m_camera_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    QList<QCameraInfo> m_cameras;

    VideoWidget* m_video = nullptr;
    VideoOverlayWidget* m_overlay;

    std::atomic<bool> m_snapshots_allowed;
    std::mutex m_camera_lock;


    std::deque<int> m_height_history;
    std::set<int> m_recent_heights;
};


}
#endif
