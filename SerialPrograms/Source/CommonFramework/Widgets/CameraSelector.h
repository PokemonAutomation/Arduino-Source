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
#include <condition_variable>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "CommonFramework/Tools/VideoFeed.h"
#include "VideoOverlay.h"

namespace PokemonAutomation{


class CameraSelectorUI;


class CameraSelector{
    static const QString JSON_CAMERA;
    static const QString JSON_RESOLUTION;

public:
    CameraSelector(QString label);
    CameraSelector(QString label, const QJsonValue& json);

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    CameraSelectorUI* make_ui(QWidget& parent, QWidget& holder);

private:
    friend class CameraSelectorUI;
    QString m_label;
    QCameraInfo m_camera;
    QSize m_resolution;
};


class CameraSelectorUI : public QWidget, public VideoFeed{
public:
    CameraSelectorUI(QWidget& parent, CameraSelector& value, QWidget& holder);

    VideoOverlay& overlay(){ return *m_overlay; }

    void set_camera_enabled(bool enabled);
    void set_resolution_enabled(bool enabled);
    void set_snapshots_allowed(bool enabled);
    void set_overlay_enabled(bool enabled);

    void update_size();
    void reset_video();

    virtual QImage snapshot() override;
    virtual void operator+=(const InferenceBox& box) override;
    virtual void operator-=(const InferenceBox& box) override;
//    virtual void test_draw() override;

private:
    void refresh();
    static QString aspect_ratio(const QSize& size);

private:
    CameraSelector& m_value;
    QWidget& m_holder;

    QComboBox* m_camera_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    QList<QCameraInfo> m_cameras;
    QList<QSize> m_resolutions;

//    QWidget* m_video;
    QCamera* m_camera;
    QCameraViewfinder* m_camera_view;
    QCameraImageCapture* m_capture;
    VideoOverlay* m_overlay;

    std::atomic<bool> m_snapshots_allowed;
    std::mutex m_camera_lock;
    std::condition_variable m_cv;
    bool m_capture_done;
    QImage m_capture_image;

    std::deque<int> m_height_history;
    std::set<int> m_recent_heights;
};


}
#endif
