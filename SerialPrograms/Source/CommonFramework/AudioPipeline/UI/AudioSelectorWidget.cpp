/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudio>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "AudioDisplayWidget.h"
#include "AudioSelectorWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



AudioSelectorWidget::~AudioSelectorWidget(){
    m_session.remove_state_listener(*this);
}

AudioSelectorWidget::AudioSelectorWidget(QWidget& parent, AudioSession& session)
    : QWidget(&parent)
    , m_session(session)
//    , m_slider_active(false)
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

        m_audio_input_box = new NoWheelComboBox(this);
        m_audio_input_box->setMaxVisibleItems(20);
        input_layout->addWidget(m_audio_input_box, 10);
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
        m_audio_output_box = new NoWheelComboBox(this);
        m_audio_output_box->setMaxVisibleItems(20);
        if (GlobalSettings::instance().AUDIO_PIPELINE->SHOW_RECORD_FREQUENCIES){
            output_layout->addWidget(m_audio_output_box, 7);
            m_record_button = new QPushButton("Record Frequencies", this);
            output_layout->addWidget(m_record_button, 3);
        }else{
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

    refresh_all();

    connect(
        m_audio_input_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index <= 0 || index >= (int)m_input_audios.size() + 2){
                m_session.clear_audio_input();
            }else if (index == 1){
                std::string path = QFileDialog::getOpenFileName(this, "Open audio file", ".", "*.wav *.mp3").toStdString();
                if (path.empty()){
                    m_session.clear_audio_input();
                }else{
                    m_session.set_audio_input(std::move(path));
                }
            }else{
                m_session.set_audio_input(m_input_audios[index - 2]);
            }
        }
    );
    connect(
        m_audio_format_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index < 0 || index >= (int)m_input_formats.size()){
                return;
            }
            m_session.set_format(m_input_formats[index]);
        }
    );
    connect(
        m_audio_output_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index <= 0 || index >= (int)m_output_audios.size() + 1){
                m_session.clear_audio_output();
            }else{
                m_session.set_audio_output(m_output_audios[index - 1]);
            }
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            m_session.reset();
            refresh_all();
        }
    );
    connect(
        m_audio_vis_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index < 0 || index > 2){
                index = 0;
            }
            m_session.set_display((AudioOption::AudioDisplayType)index);
        }
    );

    connect(
        m_volume_slider, &QSlider::valueChanged, this, [this](int value){
            m_session.set_volume(value / 100.);
        }
    );
#if 0
    connect(
        m_volume_slider, &QSlider::sliderMoved, this, [this](){
            m_session.set_volume(m_volume_slider->value() / 100.);
        }
    );
    connect(
        m_volume_slider, &QSlider::sliderPressed, this, [this](){
            m_slider_active.store(true, std::memory_order_release);
        }
    );
    connect(
        m_volume_slider, &QSlider::sliderReleased, this, [this](){
            m_slider_active.store(false, std::memory_order_release);
        }
    );
#endif

    // only in developer mode:
    // record audio
    if (GlobalSettings::instance().AUDIO_PIPELINE->SHOW_RECORD_FREQUENCIES){
        connect(m_record_button, &QPushButton::clicked, this, [this](bool){
            m_record_is_on = !m_record_is_on;
            m_session.spectrums().saveAudioFrequenciesToDisk(m_record_is_on);
            if (m_record_is_on){
                m_record_button->setText("Stop recording");
            }else{
                m_record_button->setText("Record Frequencies");
            }
        });
    }

    session.add_state_listener(*this);
}



void AudioSelectorWidget::build_input_list(const std::string& file, const AudioDeviceInfo& device){
    m_input_audios = AudioDeviceInfo::all_input_devices();
    m_audio_input_box->clear();
    m_audio_input_box->addItem("(none)");
    m_audio_input_box->addItem("Play Audio File");
    size_t index = file.empty() ? 0 : 1;
    for (size_t c = 0; c < m_input_audios.size(); c++){
        const AudioDeviceInfo& audio = m_input_audios[c];
        m_audio_input_box->addItem(QString::fromStdString(audio.display_name()));
        if (device == audio){
            index = c + 2;
        }
    }
    m_audio_input_box->setCurrentIndex((int)index);
}
void AudioSelectorWidget::build_output_list(const AudioDeviceInfo& device){
    m_output_audios = AudioDeviceInfo::all_output_devices();
    m_audio_output_box->clear();
    m_audio_output_box->addItem("(none)");
    size_t index = 0;
    for (size_t c = 0; c < m_output_audios.size(); c++){
        const AudioDeviceInfo& audio = m_output_audios[c];
        m_audio_output_box->addItem(QString::fromStdString(audio.display_name()));
        if (device == audio){
            index = c + 1;
        }
    }
    m_audio_output_box->setCurrentIndex((int)index);
}


void AudioSelectorWidget::refresh_all(){
    auto input = m_session.input_device();
    refresh_input_device(input.first, input.second);
    refresh_formats(input.first, input.second, m_session.input_format());
    refresh_output_device(m_session.output_device());
    refresh_volume(m_session.output_volume());
    refresh_display(m_session.display_type());
}
void AudioSelectorWidget::refresh_formats(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){
//    cout << "AudioSelectorWidget::refresh_formats()" << endl;
    if (!file.empty() || !device){
        m_audio_format_box->clear();
        return;
    }
//    cout << "AudioSelectorWidget::refresh_formats() - inside" << endl;

    m_input_formats = device.supported_formats();
    m_audio_format_box->clear();
    int index = -1;
    for (size_t c = 0; c < m_input_formats.size(); c++){
        m_audio_format_box->addItem(AUDIO_FORMAT_LABELS[(size_t)m_input_formats[c]]);
        if (format == m_input_formats[c]){
            index = (int)c;
        }
    }
    m_audio_format_box->setCurrentIndex(index);
}
void AudioSelectorWidget::refresh_input_device(const std::string& file, const AudioDeviceInfo& device){
    if (m_input_audios.empty()){
        build_input_list(file, device);
        return;
    }

    if (!file.empty()){
        m_audio_input_box->setCurrentIndex(1);
        return;
    }

    //  See if it's in our cached list.
    for (size_t c = 0; c < m_input_audios.size(); c++){
        if (device == m_input_audios[c]){
            m_audio_input_box->setCurrentIndex((int)c + 2);
            return;
        }
    }

    build_input_list(file, device);
}
void AudioSelectorWidget::refresh_output_device(const AudioDeviceInfo& device){
    if (m_output_audios.empty()){
        build_output_list(device);
        return;
    }

    //  See if it's in our cached list.
    for (size_t c = 0; c < m_output_audios.size(); c++){
        if (device == m_output_audios[c]){
            m_audio_output_box->setCurrentIndex((int)c + 1);
            return;
        }
    }

    build_output_list(device);
}
void AudioSelectorWidget::refresh_volume(double volume){
    m_volume_slider->setValue((int)(volume * 100));
}
void AudioSelectorWidget::refresh_display(AudioOption::AudioDisplayType display){
    switch(display){
    case AudioOption::AudioDisplayType::NO_DISPLAY:
        m_audio_vis_box->setCurrentIndex(0);
        break;
    case AudioOption::AudioDisplayType::FREQ_BARS:
        m_audio_vis_box->setCurrentIndex(1);
        break;
    case AudioOption::AudioDisplayType::SPECTROGRAM:
        m_audio_vis_box->setCurrentIndex(2);
        break;
    default:
        m_audio_vis_box->setCurrentIndex(0);
    }
}


void AudioSelectorWidget::post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){
//    cout << "AudioSelectorWidget::input_changed()" << endl;
    QMetaObject::invokeMethod(this, [this, device, file, format]{
        refresh_input_device(file, device);
        refresh_formats(file, device, format);
    });
}
void AudioSelectorWidget::post_output_change(const AudioDeviceInfo& device){
    QMetaObject::invokeMethod(this, [this, device]{
        refresh_output_device(device);
    });
}
void AudioSelectorWidget::post_volume_change(double volume){
//    if (m_slider_active.load(std::memory_order_acquire)){
//        return;
//    }
    QMetaObject::invokeMethod(this, [this]{
//        refresh_volume(volume);
        refresh_volume(m_session.output_volume());
    }, Qt::QueuedConnection);   //  Queued due to potential recursive call to the same lock.
}
void AudioSelectorWidget::post_display_change(AudioOption::AudioDisplayType display){
    QMetaObject::invokeMethod(this, [this]{
        refresh_display(m_session.display_type());
    }, Qt::QueuedConnection);   //  Queued due to potential recursive call to the same lock.
}






}
