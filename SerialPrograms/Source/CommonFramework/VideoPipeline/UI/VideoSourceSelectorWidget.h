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



class VideoSourceSelectorWidget : public QWidget, public VideoSession::StateListener{
public:
    ~VideoSourceSelectorWidget();
    VideoSourceSelectorWidget(Logger& logger, VideoSession& session);


private:
    void update_source_list();
    void update_resolution_list();

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
