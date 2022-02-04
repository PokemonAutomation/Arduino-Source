/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/Qt/NoWheelComboBox.h"
#include "AudioDisplayWidget.h"
#include "AudioSelectorWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"

#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#elif QT_VERSION_MAJOR == 6
#include <QMediaDevices>
#include <QAudioDevice>
#endif

#include <iostream>
#include <tuple>

namespace PokemonAutomation{

// Get all system audio input devices and their human readable descriptions.
std::tuple<std::vector<AudioInfo>, std::vector<QString>> get_all_audio_inputs(){
    std::tuple<std::vector<AudioInfo>, std::vector<QString>> ret;
#if QT_VERSION_MAJOR == 5
    const auto audioInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (const auto &audioInfo : audioInfos){
        std::get<0>(ret).emplace_back(audioInfo.deviceName().toStdString());
        std::get<1>(ret).emplace_back(audioInfo.deviceName());
    }
#elif QT_VERSION_MAJOR == 6
    const auto audioInfos = QMediaDevices::audioInputs();
    for (const auto &audioInfo : audioInfos){
        std::get<0>(ret).emplace_back(audioInfo.id().toStdString());
        std::get<1>(ret).emplace_back(audioInfo.description());
    }
#endif

    return ret;
}

// Get all system audio output devices and their human readable descriptions.
std::tuple<std::vector<AudioInfo>, std::vector<QString>> get_all_audio_outputs(){
    std::tuple<std::vector<AudioInfo>, std::vector<QString>> ret;
#if QT_VERSION_MAJOR == 5
    const auto audioInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (const auto &audioInfo : audioInfos){
        std::get<0>(ret).emplace_back(audioInfo.deviceName().toStdString());
        std::get<1>(ret).emplace_back(audioInfo.deviceName());
    }
#elif QT_VERSION_MAJOR == 6
    const auto audioInfos = QMediaDevices::audioOutputs();
    for (const auto &audioInfo : audioInfos){
        std::get<0>(ret).emplace_back(audioInfo.id().toStdString());
        std::get<1>(ret).emplace_back(audioInfo.description());
    }
#endif

    return ret;
}

AudioSelectorWidget::~AudioSelectorWidget(){}

AudioSelectorWidget::AudioSelectorWidget(
    QWidget& parent,
    Logger& logger,
    AudioSelector& value,
    AudioDisplayWidget& holder
)
    : QWidget(&parent)
    , m_logger(logger)
    , m_value(value)
    , m_display(holder)
{
    QHBoxLayout* audio_row = new QHBoxLayout(this);
    audio_row->setContentsMargins(0, 0, 0, 0);

    audio_row->addWidget(new QLabel("<b>Audio Input:</b>", this), 1);
    audio_row->addSpacing(5);

    m_audio_input_box = new NoWheelComboBox(this);
    audio_row->addWidget(m_audio_input_box, 5);
    audio_row->addSpacing(5);

    audio_row->addWidget(new QLabel("<b>Audio Output:</b>", this), 1);
    m_audio_output_box = new NoWheelComboBox(this);
    audio_row->addWidget(m_audio_output_box, 5);
    audio_row->addSpacing(5);

    m_audio_vis_box = new NoWheelComboBox(this);
    audio_row->addWidget(m_audio_vis_box, 5);
    audio_row->addSpacing(5);
    m_audio_vis_box->addItem("No Audio Display");
    m_audio_vis_box->addItem("Frequency Bars");
    m_audio_vis_box->addItem("Spectrogram");

    refresh();

    m_reset_button = new QPushButton("Reset Audio", this);
    audio_row->addWidget(m_reset_button, 1);

    connect(
        m_audio_input_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            AudioInfo& current = m_value.m_inputDevice;
            if (index <= 0 || index > (int)m_input_audios.size()){
                current = AudioInfo();
            }else{
                const AudioInfo& audio = m_input_audios[index - 1];
                if (current == audio){
                    return;
                }
                current = audio;
            }
            reset_audio();
        }
    );
    connect(
        m_audio_output_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            AudioInfo& current = m_value.m_outputDevice;
            if (index <= 0 || index > (int)m_output_audios.size()){
                current = AudioInfo();
            }else{
                const AudioInfo& audio = m_output_audios[index - 1];
                if (current == audio){
                    return;
                }
                current = audio;
            }
            reset_audio();
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
            reset_audio();
        }
    );
    connect(
        this, &AudioSelectorWidget::internal_async_reset_audio,
        this, [=]{
            reset_audio();
        }
    );
    connect(
        m_audio_vis_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            switch(index){
                case 0:
                    m_value.m_audioDisplayType = AudioSelector::AudioDisplayType::NO_DISPLAY;
                    break;
                case 1:
                    m_value.m_audioDisplayType = AudioSelector::AudioDisplayType::FREQ_BARS;
                    break;
                case 2:
                    m_value.m_audioDisplayType = AudioSelector::AudioDisplayType::SPECTROGRAM;
                    break;
                default:
                    m_value.m_audioDisplayType = AudioSelector::AudioDisplayType::NO_DISPLAY;
            }
            m_display.setAudioDisplayType(m_value.m_audioDisplayType);
        }
    );

    // only in developer mode:
    // record audio
    if (GlobalSettings::instance().DEVELOPER_MODE){
        m_record_button = new QPushButton("Record Frequencies", this);
        audio_row->addWidget(m_record_button, 1);

        connect(m_record_button, &QPushButton::clicked, this, [=](bool){
            m_record_is_on = !m_record_is_on;
            m_display.saveAudioFrequenciesToDisk(m_record_is_on);
            if (m_record_is_on){
                m_record_button->setText("Stop recording");
            } else{
                m_record_button->setText("Record Frequencies");
            }
        });
    }
}
void AudioSelectorWidget::refresh(){

    auto build_device_menu = [](
            QComboBox* box,
            const std::vector<AudioInfo>& audios,
            const std::vector<QString>& descriptions,
            AudioInfo& current_device){
        
        box->clear();
        box->addItem("(none)");

        size_t index = 0;
        for (size_t c = 0; c < audios.size(); c++){
            const AudioInfo& audio = audios[c];
            box->addItem(descriptions[c]);

            if (current_device == audio){
                index = c + 1;
            }
        }
        if (index != 0){
            box->setCurrentIndex((int)index);
        }else{
            current_device = AudioInfo();
            box->setCurrentIndex(0);
        }
    };

    std::vector<QString> descriptions;
    std::tie(m_input_audios, descriptions) = get_all_audio_inputs();
    build_device_menu(m_audio_input_box, m_input_audios, descriptions, m_value.m_inputDevice);

    std::tie(m_output_audios, descriptions) = get_all_audio_outputs();
    build_device_menu(m_audio_output_box, m_output_audios, descriptions, m_value.m_outputDevice);

    // std::cout << "Refresh: " << AudioSelector::audioDisplayTypeToString(m_value.m_audioDisplayType) << std::endl;
    switch(m_value.m_audioDisplayType){
        case AudioSelector::AudioDisplayType::NO_DISPLAY:
            m_audio_vis_box->setCurrentIndex(0);
            break;
        case AudioSelector::AudioDisplayType::FREQ_BARS:
            m_audio_vis_box->setCurrentIndex(1);
            break;
        case AudioSelector::AudioDisplayType::SPECTROGRAM:
            m_audio_vis_box->setCurrentIndex(2);
            break;
        default:
            m_audio_vis_box->setCurrentIndex(0);
    }
    m_display.setAudioDisplayType(m_value.m_audioDisplayType);
}

void AudioSelectorWidget::reset_audio(){
    std::lock_guard<std::mutex> lg(m_audio_lock);
    m_display.close_audio();

    const AudioInfo& info = m_value.m_inputDevice;
    if (info){
        m_display.set_audio(m_logger, info, m_value.m_outputDevice);
    }
}
void AudioSelectorWidget::async_reset_audio(){
    emit internal_async_reset_audio();
}


void AudioSelectorWidget::spectrums_since(size_t startingStamp, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums){
    m_display.spectrums_since(startingStamp, spectrums);
}

void AudioSelectorWidget::spectrums_latest(size_t numLatestSpectrums, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums){
    m_display.spectrums_latest(numLatestSpectrums, spectrums);
}

void AudioSelectorWidget::set_audio_enabled(bool enabled){
    m_audio_input_box->setEnabled(enabled);
}



}
