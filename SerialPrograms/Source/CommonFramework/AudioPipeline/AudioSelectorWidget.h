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
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "AudioOption.h"

class QComboBox;
class QPushButton;

namespace PokemonAutomation{


class AudioSelectorWidget : public QWidget{
public:
    AudioSelectorWidget(
        QWidget& parent,
        LoggerQt& logger,
        AudioSession& session
    );
    ~AudioSelectorWidget();

private:
    void update_formats();
    void refresh();

private:
    LoggerQt& m_logger;
    AudioSession& m_session;

    QComboBox* m_audio_input_box = nullptr;
    QComboBox* m_audio_format_box = nullptr;
    QComboBox* m_audio_output_box = nullptr;
    QComboBox* m_audio_vis_box = nullptr;

    QSlider* m_volume_slider = nullptr;
    
    QPushButton* m_reset_button = nullptr;
    QPushButton* m_load_file_button = nullptr;
    std::string m_absoluteFilepath;
    QPushButton* m_record_button = nullptr;
    bool m_record_is_on = false;

    std::vector<AudioDeviceInfo> m_input_audios;
    std::vector<AudioDeviceInfo> m_output_audios;

    std::mutex m_audio_lock;
};


}
#endif
