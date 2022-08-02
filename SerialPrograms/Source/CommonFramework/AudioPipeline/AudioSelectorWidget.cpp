/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QAudio>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include "Common/Qt/NoWheelComboBox.h"
#include "AudioDisplayWidget.h"
#include "AudioSelectorWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


// Slider bar volume: [0, 100], in log scale
// Volume value passed to AudioDisplayWidget (and the audio thread it manages): [0.f, 1.f], linear scale
float convertAudioVolumeFromSlider(int volume){
    // The slider bar value is in the log scale because log scale matches human sound
    // perception.
    float linearVolume = QAudio::convertVolume(volume / float(100.0f),
        QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
    return linearVolume;
}

AudioSelectorWidget::~AudioSelectorWidget(){}

AudioSelectorWidget::AudioSelectorWidget(
    QWidget& parent,
    LoggerQt& logger,
    AudioSelector& value,
    AudioDisplayWidget& holder
)
    : QWidget(&parent)
    , m_logger(logger)
    , m_value(value)
    , m_display(holder)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    {
        QHBoxLayout* row0 = new QHBoxLayout();
        row0->setContentsMargins(0, 0, 0, 0);
        vbox->addLayout(row0);

        row0->addWidget(new QLabel("<b>Audio Input:</b>", this), 2);
        row0->addSpacing(5);

        QHBoxLayout* input_layout = new QHBoxLayout();
        row0->addLayout(input_layout, 10);

        if (PreloadSettings::instance().DEVELOPER_MODE){
            m_audio_input_box = new NoWheelComboBox(this);
            input_layout->addWidget(m_audio_input_box, 7);
            m_load_file_button = new QPushButton("Load File", this);
            input_layout->addWidget(m_load_file_button, 3);
        }else{
            m_audio_input_box = new NoWheelComboBox(this);
            input_layout->addWidget(m_audio_input_box, 10);
        }
        row0->addSpacing(5);

        m_audio_format_box = new NoWheelComboBox(this);
        row0->addWidget(m_audio_format_box, 6);
        row0->addSpacing(5);

        m_reset_button = new QPushButton("Reset Audio", this);
        row0->addWidget(m_reset_button, 2);
    }

    {
        QHBoxLayout* row1 = new QHBoxLayout();
        row1->setContentsMargins(0, 0, 0, 0);
        vbox->addLayout(row1);

        row1->addWidget(new QLabel("<b>Audio Output:</b>", this), 2);
        row1->addSpacing(5);

        QHBoxLayout* output_layout = new QHBoxLayout();
        row1->addLayout(output_layout, 10);
        if (PreloadSettings::instance().DEVELOPER_MODE){
            m_audio_output_box = new NoWheelComboBox(this);
            output_layout->addWidget(m_audio_output_box, 7);
            m_record_button = new QPushButton("Record Frequencies", this);
            output_layout->addWidget(m_record_button, 3);
        }else{
            m_audio_output_box = new NoWheelComboBox(this);
            output_layout->addWidget(m_audio_output_box, 10);
        }
        row1->addSpacing(5);

        m_volume_slider = new QSlider(Qt::Horizontal, this);
        m_volume_slider->setRange(0, 100);
        m_volume_slider->setTickInterval(10);
        m_volume_slider->setMinimumWidth(40);
        m_volume_slider->setTickPosition(QSlider::TicksBothSides);
        row1->addWidget(m_volume_slider, 4);
        row1->addSpacing(5);

        m_audio_vis_box = new NoWheelComboBox(this);
        m_audio_vis_box->addItem("No Display");
        m_audio_vis_box->addItem("Spectrum");
        m_audio_vis_box->addItem("Spectrogram");
        row1->addWidget(m_audio_vis_box, 4);
    }

    refresh();

    connect(
        m_audio_input_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            AudioDeviceInfo& current = m_value.m_inputDevice;
            if (index <= 0 || index > (int)m_input_audios.size()){
                current = AudioDeviceInfo();
            }else{
                // The user selects an audio source. If previously the user was playing an
                // audio file, then we shouldn't play the file any more.
                // Clear m_absoluteFilepath here so that m_display will be initialized to load
                // from audio source instead of a file.
                m_absoluteFilepath = "";
                const AudioDeviceInfo& audio = m_input_audios[index - 1];
                if (current == audio){
                    return;
                }
                current = audio;
            }
            m_value.m_inputFormat = AudioFormat::NONE;
            update_formats();
            reset_audio();
        }
    );
    connect(
        m_audio_format_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            AudioFormat& current = m_value.m_inputFormat;
            const std::vector<AudioFormat>& supported_formats = m_value.m_inputDevice.supported_formats();
            if (supported_formats.empty()){
                current = AudioFormat::NONE;
            }else{
                index = std::max(index, 0);
                index = std::min(index, (int)supported_formats.size() - 1);
                current = supported_formats[index];
            }
            reset_audio();
        }
    );
    connect(
        m_audio_output_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            AudioDeviceInfo& current = m_value.m_outputDevice;
            if (index <= 0 || index > (int)m_output_audios.size()){
                current = AudioDeviceInfo();
            }else{
                const AudioDeviceInfo& audio = m_output_audios[index - 1];
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

    connect(
        m_volume_slider, &QSlider::valueChanged, this, [=](){
            m_value.m_volume = m_volume_slider->value();
            m_value.m_volume = std::max(std::min(m_value.m_volume, 100), 0);
            emit m_display.volumeChanged(convertAudioVolumeFromSlider(m_value.m_volume));
        }
    );

    // only in developer mode:
    // record audio
    if (PreloadSettings::instance().DEVELOPER_MODE){
        connect(m_record_button, &QPushButton::clicked, this, [=](bool){
            m_record_is_on = !m_record_is_on;
            m_display.state().saveAudioFrequenciesToDisk(m_record_is_on);
            if (m_record_is_on){
                m_record_button->setText("Stop recording");
            } else{
                m_record_button->setText("Record Frequencies");
            }
        });

        connect(m_load_file_button,  &QPushButton::clicked, this, [=](bool){
            m_absoluteFilepath = QFileDialog::getOpenFileName(this, tr("Open audio file"), ".", "*.wav *.mp3").toStdString();
            std::cout << "Select file " << m_absoluteFilepath << std::endl;
            if (m_absoluteFilepath.size() > 0){
                if (m_audio_input_box->currentIndex() != 0) {
                    m_audio_input_box->setCurrentIndex(0);
                    // We connect index changed signal to a lambda function which calls reset_audio().
                    // So we don't call reset_audio() here. 
                } else{
                    reset_audio();
                }
            }
        });
    }
}
void AudioSelectorWidget::update_formats(){
    const AudioDeviceInfo& device = m_value.m_inputDevice;
    AudioFormat current_format = m_value.m_inputFormat;

    m_audio_format_box->clear();
    const std::vector<AudioFormat>& supported_formats = device.supported_formats();

    if (supported_formats.empty()){
        m_value.m_inputFormat = AudioFormat::NONE;
        return;
    }

    int index = device.preferred_format_index();
    for (size_t c = 0; c < supported_formats.size(); c++){
        m_audio_format_box->addItem(AUDIO_FORMAT_LABELS[(size_t)supported_formats[c]]);
        if (current_format == supported_formats[c]){
            index = (int)c;
        }
    }

    index = std::max(index, 0);

    m_value.m_inputFormat = supported_formats[index];
    m_audio_format_box->setCurrentIndex(index);
}
void AudioSelectorWidget::refresh(){

    auto build_device_menu = [](
        QComboBox* box,
        const std::vector<AudioDeviceInfo>& list,
        AudioDeviceInfo& current_device
    ){
        
        box->clear();
        box->addItem("(none)");

        size_t index = 0;
        for (size_t c = 0; c < list.size(); c++){
            const AudioDeviceInfo& audio = list[c];
            box->addItem(QString::fromStdString(list[c].display_name()));

            if (current_device == audio){
                index = c + 1;
            }
        }
        if (index != 0){
            box->setCurrentIndex((int)index);
        }else{
            current_device = AudioDeviceInfo();
            box->setCurrentIndex(0);
        }
    };


    m_input_audios = AudioDeviceInfo::all_input_devices();
    m_output_audios = AudioDeviceInfo::all_output_devices();
    build_device_menu(m_audio_input_box, m_input_audios, m_value.m_inputDevice);
    build_device_menu(m_audio_output_box, m_output_audios, m_value.m_outputDevice);

    update_formats();

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

    m_volume_slider->setValue(m_value.m_volume);
}

void AudioSelectorWidget::reset_audio(){
    m_logger.log("reset audio: " + m_absoluteFilepath);
    std::lock_guard<std::mutex> lg(m_audio_lock);
    m_display.close_audio();

    const AudioDeviceInfo& info = m_value.m_inputDevice;
    if ((info && m_value.m_inputFormat != AudioFormat::NONE) || m_absoluteFilepath.size() > 0){
        m_display.set_audio(
            m_logger,
            info,
            m_value.m_inputFormat,
            m_absoluteFilepath,
            m_value.m_outputDevice,
            convertAudioVolumeFromSlider(m_value.m_volume)
        );
    }
}
void AudioSelectorWidget::async_reset_audio(){
    emit internal_async_reset_audio();
}

std::vector<AudioSpectrum> AudioSelectorWidget::spectrums_since(size_t startingStamp){
    return m_display.state().spectrums_since(startingStamp);
}

std::vector<AudioSpectrum> AudioSelectorWidget::spectrums_latest(size_t numLatestSpectrums){
    return m_display.state().spectrums_latest(numLatestSpectrums);
}

void AudioSelectorWidget::add_overlay(size_t startingStamp, size_t endStamp, Color color){
    m_display.add_overlay(startingStamp, endStamp, color);
}

void AudioSelectorWidget::set_audio_enabled(bool enabled){
    m_audio_input_box->setEnabled(enabled);
}

}
