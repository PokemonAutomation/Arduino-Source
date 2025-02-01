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
#include "CommonFramework/VideoPipeline/CameraSession.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{

class Logger;
class VideoDisplayWidget;


//  Widget to handle UI that selects video source and resolution.
//  The video state is managed by a class CameraSelector
//  object passed in the constructor.
//  CameraSelectorWidget is also responsible for updating
//  VideoDisplayWidget, the video feed UI, when the video state
//  is changed. The VideoDisplayWidget is passed in the constructor
//  too.
class CameraSelectorWidget : public QWidget, public CameraSession::StateListener{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT
public:
    CameraSelectorWidget(
        CameraSession& session,
        Logger& logger,
        VideoDisplayWidget& holder
    );
    ~CameraSelectorWidget();

private:
    void update_camera_list();
    void update_resolution_list();

    virtual void pre_shutdown() override;
    virtual void post_new_source(const CameraInfo& device, Resolution resolution) override;
    virtual void post_resolution_change(Resolution resolution) override;

private:
    Logger& m_logger;
    CameraSession& m_session;

    VideoDisplayWidget& m_display;
    std::vector<Resolution> m_resolutions;

    QComboBox* m_camera_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    std::vector<CameraInfo> m_cameras;

//    std::atomic<bool> m_snapshots_allowed;
    std::mutex m_camera_lock;
};


}
#endif
