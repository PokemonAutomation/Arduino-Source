/*  Camera Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraSelectorWidget_H
#define PokemonAutomation_VideoPipeline_CameraSelectorWidget_H

#include <vector>
#include <atomic>
#include <mutex>
#include <QWidget>
#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CameraOption.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{

//  Widget to handle UI that selects video source and resolution.
//  The video state is managed by a class CameraSelector
//  object passed in the constructor.
//  CameraSelectorWidget is also responsible for updating
//  VideoDisplayWidget, the video feed UI, when the video state
//  is changed. The VideoDisplayWidget is passed in the constructor
//  too.
//  CameraSelectorWidget inherits VideoFeed to provide function
//  async_reset_video() and snapshot().
class CameraSelectorWidget : public QWidget, public VideoFeed{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT
public:
    CameraSelectorWidget(
        QWidget& parent,
        LoggerQt& logger,
        CameraOption& value,
        VideoDisplayWidget& holder
    );
    ~CameraSelectorWidget();

    void set_camera_enabled(bool enabled);
    void set_resolution_enabled(bool enabled);
    void set_snapshots_allowed(bool enabled);
    void set_overlay_enabled(bool enabled);

    void reset_video();

    //  Override VideoFeed::async_reset_video().
    //  Can be called from any thread.
    virtual void async_reset_video() override;

    //  Override VideoFeed::snapshot().
    //  Cannot be called from UI thread.
    virtual VideoSnapshot snapshot() override;

signals:
    // Need to define this version of async_reset_video()
    // because it will be used as a signal.
    void internal_async_reset_video();

private:
    void refresh();

private:
    LoggerQt& m_logger;
    CameraOption& m_value;

    VideoDisplayWidget& m_display;
    std::vector<Resolution> m_resolutions;

    QComboBox* m_camera_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    std::vector<CameraInfo> m_cameras;

    std::atomic<bool> m_snapshots_allowed;
    std::mutex m_camera_lock;
};


}
#endif
