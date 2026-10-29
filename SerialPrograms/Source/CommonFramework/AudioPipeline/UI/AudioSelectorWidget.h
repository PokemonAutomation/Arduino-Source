/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSelectorWidget_H
#define PokemonAutomation_AudioPipeline_AudioSelectorWidget_H

#include <string>
#include <vector>
#include <QWidget>
#include <QSlider>
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{


class AudioSelectorWidget : public QWidget, private AudioSession::StateListener{
public:
    AudioSelectorWidget(QWidget& parent, AudioSession& session);
    ~AudioSelectorWidget();

private:
    void build_output_list(const AudioDeviceInfo& device);

    void refresh_all();
    void refresh_formats(const std::shared_ptr<const AudioSourceDescriptor>& source, const AudioInputChoice& choice);
    void refresh_input_device(const std::shared_ptr<const AudioSourceDescriptor>& source);
    void refresh_output_device(const AudioDeviceInfo& device);
    void refresh_volume(double volume);
    void refresh_display(AudioOption::AudioDisplayType display);

    virtual void post_input_change(
        const std::shared_ptr<const AudioSourceDescriptor>& source,
        const AudioInputChoice& choice
    ) override;
    virtual void post_output_change(const AudioDeviceInfo& device) override;
    virtual void post_volume_change(double volume) override;
    virtual void post_display_change(AudioOption::AudioDisplayType display) override;

private:
    AudioSession& m_session;

    QComboBox* m_audio_input_box = nullptr;
    QComboBox* m_audio_format_box = nullptr;
    QComboBox* m_audio_output_box = nullptr;
    QComboBox* m_audio_vis_box = nullptr;

    QSlider* m_volume_slider = nullptr;
    
    QPushButton* m_reset_button = nullptr;
//    QPushButton* m_load_file_button = nullptr;
    std::string m_absoluteFilepath;
    QPushButton* m_record_button = nullptr;
    bool m_record_is_on = false;

    std::vector<std::shared_ptr<AudioSourceDescriptor>> m_input_sources;
    std::vector<AudioInputChoice> m_input_choices;

    std::vector<AudioDeviceInfo> m_output_audios;

//    Mutex m_audio_lock;
//    std::atomic<bool> m_slider_active;
};


}
#endif
