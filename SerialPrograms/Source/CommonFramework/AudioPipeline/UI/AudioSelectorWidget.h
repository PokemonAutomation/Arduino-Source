/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSelectorWidget_H
#define PokemonAutomation_AudioPipeline_AudioSelectorWidget_H

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <QWidget>
#include <QSlider>
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{


class AudioSelectorWidget : public QWidget, private AudioSession::Listener{
public:
    AudioSelectorWidget(
        QWidget& parent,
        Logger& logger,
        AudioSession& session
    );
    ~AudioSelectorWidget();

private:
    void build_input_list(const std::string& file, const AudioDeviceInfo& device);
    void build_output_list(const AudioDeviceInfo& device);

    void refresh_all();
    void refresh_formats(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format);
    void refresh_input_device(const std::string& file, const AudioDeviceInfo& device);
    void refresh_output_device(const AudioDeviceInfo& device);
    void refresh_volume(double volume);
    void refresh_display(AudioOption::AudioDisplayType display);

    virtual void input_changed(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format) override;
    virtual void output_changed(const AudioDeviceInfo& device) override;
    virtual void volume_changed(double volume) override;
    virtual void display_changed(AudioOption::AudioDisplayType display) override;

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

    std::vector<AudioDeviceInfo> m_input_audios;
    std::vector<AudioChannelFormat> m_input_formats;

    std::vector<AudioDeviceInfo> m_output_audios;

    std::mutex m_audio_lock;
//    std::atomic<bool> m_slider_active;
};


}
#endif
