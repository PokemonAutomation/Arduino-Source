/*  Video Source Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoSourceSelectorWidget_H
#define PokemonAutomation_VideoPipeline_VideoSourceSelectorWidget_H

#include <vector>
#include <QWidget>
#include "CommonFramework/VideoPipeline/VideoSession.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{


//  UI to select what video source to use for an Switch automation program
//  This class inherits VideoSession::StateListener so it has callbacks for state change
//  in the video session.
class VideoSourceSelectorWidget : public QWidget, public VideoSession::StateListener{
public:
    ~VideoSourceSelectorWidget();
    VideoSourceSelectorWidget(Logger& logger, VideoSession& session);


private:
    //  Query the OS to get all the possible video sources and update the UI to show them.
    void update_source_list();
    //  Load all possible video resolutions from the current video source.
    void update_resolution_list();

    //  Overwrites VideoSession::StateListener::post_startup().
    //  This function is called after setting a new video source or resetting the
    //  existing video source.
    //  This also includes after the video resolution is changed as we need to rebuild the video
    //  source with the new resolution.
    virtual void post_startup(VideoSource* source) override;


private:
    Logger& m_logger;
    VideoSession& m_session;

    QComboBox* m_sources_box;
    QComboBox* m_resolution_box;
    QPushButton* m_reset_button;

    std::vector<std::shared_ptr<VideoSourceDescriptor>> m_sources;
    std::vector<Resolution> m_resolutions;

};



}
#endif
