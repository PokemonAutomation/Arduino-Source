/*  Video Source Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
#include "CommonFramework/Panels/ConsoleSettingsStretch.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "VideoSourceSelectorWidget.h"

#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Null.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_Camera.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


VideoSourceSelectorWidget::~VideoSourceSelectorWidget(){
    m_session.remove_state_listener(*this);
}
VideoSourceSelectorWidget::VideoSourceSelectorWidget(Logger& logger, VideoSession& session)
    : m_logger(logger)
    , m_session(session)
{
    QHBoxLayout* layout0 = new QHBoxLayout(this);
    layout0->setContentsMargins(0, 0, 0, 0);

    layout0->addWidget(new QLabel("<b>Video Input:</b>", this), CONSOLE_SETTINGS_STRETCH_L0_LABEL);

    QHBoxLayout* layout1 = new QHBoxLayout();
    layout0->addLayout(layout1, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
    layout1->setContentsMargins(0, 0, 0, 0);

    m_sources_box = new NoWheelCompactComboBox(this);
    m_sources_box->setMaxVisibleItems(20);
    layout1->addWidget(m_sources_box, CONSOLE_SETTINGS_STRETCH_L1_BODY);
    layout1->addSpacing(5);

    {
        QHBoxLayout* layout2 = new QHBoxLayout();
        layout1->addLayout(layout2, CONSOLE_SETTINGS_STRETCH_L1_RIGHT);
        layout2->setContentsMargins(0, 0, 0, 0);

        m_resolution_box = new NoWheelCompactComboBox(this);
        m_resolution_box->setMaxVisibleItems(20);
        layout2->addWidget(m_resolution_box, 2);
        layout2->addSpacing(5);

        m_format_box = new NoWheelCompactComboBox(this);
        m_format_box->setMaxVisibleItems(20);
        layout2->addWidget(m_format_box, 1);
    }
    layout1->addSpacing(5);

    m_reset_button = new QPushButton("Reset Video", this);
    layout1->addWidget(m_reset_button, CONSOLE_SETTINGS_STRETCH_L1_BUTTON);

    update_source_list();
    update_resolution_list();

    // Set the action for the video source selection box
    connect(
        m_sources_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (0 <= index && index < (int)m_sources.size()){
                m_sources[index]->run_post_select();
                m_session.set_source(m_sources[index]);
            }else{
                m_session.set_source(std::make_unique<VideoSourceDescriptor_Null>());
            }
        }
    );

    // Set the action for the video resolution selection box
    connect(
        m_resolution_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index < 0 || index >= (int)m_formats.size()){
                return;
            }
            Resolution resolution = m_formats[index].first;
            m_session.set_resolution(resolution);
        }
    );

    // Set the action for the video format selection box
    connect(
        m_format_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            int resolution_index = m_resolution_box->currentIndex();
            if (resolution_index < 0 || resolution_index > (int)m_formats.size()){
                return;
            }
            const std::vector<std::pair<VideoFormat, size_t>>& format_list = m_formats[resolution_index].second;
            if (index < 0 || index >= (int)format_list.size()){
                return;
            }
            const std::pair<VideoFormat, size_t>& format = format_list[index];
            m_session.set_format(format.first, format.second);
        }
    );

    //  Set the action for the video reset button
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            update_source_list();
            m_session.reset();
        }
    );

    m_session.add_state_listener(*this);
}




void VideoSourceSelectorWidget::update_source_list(){
    m_sources_box->clear();
    m_sources.clear();

    std::shared_ptr<const VideoSourceDescriptor> current_descriptor = m_session.descriptor();

    //  Add all the static options.
    {
        VideoSourceOption option;
        m_session.get(option);
        m_sources.emplace_back(option.get_descriptor_from_cache(VideoSourceType::None));
        m_sources.emplace_back(option.get_descriptor_from_cache(VideoSourceType::StillImage));
    }

    //  Now add all the cameras.
    for (const CameraInfo& info : get_all_cameras()){
        m_sources.emplace_back(std::make_unique<VideoSourceDescriptor_Camera>(info));
    }

    int index = -1;
    for (int c = 0; c < (int)m_sources.size(); c++){
        const VideoSourceDescriptor& descriptor = *m_sources[c];
        m_sources_box->addItem(QString::fromStdString(descriptor.display_name()));
        if (*current_descriptor == descriptor){
            index = c;
        }
    }
    if (index >= 0){
        m_sources_box->setCurrentIndex((int)index);
    }else{
        m_logger.log("Unable to find entry for this source.", COLOR_RED);
    }
}
void VideoSourceSelectorWidget::update_resolution_list(){
    m_resolution_box->clear();
    m_format_box->clear();
    m_formats.clear();

    Resolution current_resolution;
    VideoFormat current_format;
    FramesPerSecond current_fps;
    m_session.current_stream_format(current_resolution, current_format, current_fps);

    VideoFormatSet format_set = m_session.supported_formats();

    for (auto& resolution : format_set){
        if (resolution.second.empty()){
            continue;
        }
        std::pair<
            Resolution,
            std::vector<std::pair<VideoFormat, size_t>>
        >& entry = m_formats.emplace_back();
        entry.first = resolution.first;
        for (auto& format : resolution.second){
            for (size_t fps : format.second){
                entry.second.emplace_back(format.first, fps);
                if (!GlobalSettings::instance().VIDEO_PIPELINE->SHOW_ALL_FPS){
                    break;
                }
            }
        }
    }

    //  Make resolution box.
    int resolution_index = -1;
    {
        for (int c = 0; c < (int)m_formats.size(); c++){
            Resolution res = m_formats[c].first;
            m_resolution_box->addItem(
                QString::fromStdString(
                    std::to_string(res.width) + " x " +
                    std::to_string(res.height) + " " +
                    aspect_ratio_as_string(res)
                )
            );
            if (res == current_resolution){
                resolution_index = c;
            }
        }
        m_resolution_box->setCurrentIndex(resolution_index);

        if (resolution_index < 0){
            m_logger.log("Unable to find entry for this resolution.", COLOR_RED);
            return;
        }
    }

    //  Make format box.

    const std::vector<std::pair<VideoFormat, size_t>>& format_list = m_formats[resolution_index].second;

//    cout << "current_format = " << (int)current_format << endl;

    int format_index = -1;
    for (int c = 0; c < (int)format_list.size(); c++){
        const std::pair<VideoFormat, size_t>& format = format_list[c];

        std::string label = VideoFormat_database().find(format.first)->display;
        if (format.second != 0){
            label += " (" + std::to_string(format.second) + " fps)";
        }

        m_format_box->addItem(QString::fromStdString(label));
        if (format.first == current_format && format.second == current_fps){
            format_index = c;
        }
    }
    m_format_box->setCurrentIndex(format_index);

    if (format_index < 0){
        m_logger.log("Unable to find entry for this format.", COLOR_RED);
        return;
    }


}




void VideoSourceSelectorWidget::post_startup(VideoSource* source){
    update_source_list();
    update_resolution_list();
}








}
