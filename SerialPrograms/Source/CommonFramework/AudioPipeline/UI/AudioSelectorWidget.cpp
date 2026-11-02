/*  Audio Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Panels/ConsoleSettingsStretch.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"

#include "CommonFramework/AudioPipeline/Sources/AudioSource_Null.h"
//#include "CommonFramework/AudioPipeline/Sources/AudioSource_File.h"
#include "CommonFramework/AudioPipeline/Sources/AudioSource_Device.h"

#include "AudioDisplayWidget.h"
#include "AudioSelectorWidget.h"

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
        QHBoxLayout* layout0 = new QHBoxLayout();
        layout0->setContentsMargins(0, 0, 0, 0);
        vbox->addLayout(layout0);

        layout0->addWidget(new QLabel("<b>Audio Input:</b>", this), CONSOLE_SETTINGS_STRETCH_L0_LABEL);

        QHBoxLayout* layout1 = new QHBoxLayout();
        layout0->addLayout(layout1, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
        layout1->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout* input_layout = new QHBoxLayout();
        layout1->addLayout(input_layout, CONSOLE_SETTINGS_STRETCH_L1_BODY);
        layout1->addSpacing(5);

        m_audio_input_box = new NoWheelCompactComboBox(this);
        m_audio_input_box->setMaxVisibleItems(20);
        input_layout->addWidget(m_audio_input_box);

        m_audio_format_box = new NoWheelCompactComboBox(this);
        layout1->addWidget(m_audio_format_box, CONSOLE_SETTINGS_STRETCH_L1_RIGHT);
        layout1->addSpacing(5);

        m_reset_button = new QPushButton("Reset Audio", this);
        layout1->addWidget(m_reset_button, CONSOLE_SETTINGS_STRETCH_L1_BUTTON);
    }

    {
        QHBoxLayout* layout0 = new QHBoxLayout();
        layout0->setContentsMargins(0, 0, 0, 0);
        vbox->addLayout(layout0);

        layout0->addWidget(new QLabel("<b>Audio Output:</b>", this), CONSOLE_SETTINGS_STRETCH_L0_LABEL);

        QHBoxLayout* layout1 = new QHBoxLayout();
        layout0->addLayout(layout1, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
        layout1->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout* output_layout = new QHBoxLayout();
        layout1->addLayout(output_layout, CONSOLE_SETTINGS_STRETCH_L1_BODY);
        layout1->addSpacing(5);

        m_audio_output_box = new NoWheelCompactComboBox(this);
        m_audio_output_box->setMaxVisibleItems(20);
        if (GlobalSettings::instance().AUDIO_PIPELINE->SHOW_RECORD_FREQUENCIES){
            output_layout->addWidget(m_audio_output_box, 7);
            m_record_button = new QPushButton("Record Frequencies", this);
            output_layout->addWidget(m_record_button, 3);
        }else{
            output_layout->addWidget(m_audio_output_box, 10);
        }

        m_volume_slider = new QSlider(Qt::Horizontal, this);
        m_volume_slider->setRange(0, 100);
        m_volume_slider->setTickInterval(10);
        m_volume_slider->setMinimumWidth(40);
        m_volume_slider->setTickPosition(QSlider::TicksBothSides);
        layout1->addWidget(m_volume_slider, 2);
        layout1->addSpacing(5);

        m_audio_vis_box = new NoWheelCompactComboBox(this);
        m_audio_vis_box->addItem("No Display");
        m_audio_vis_box->addItem("Spectrum");
        m_audio_vis_box->addItem("Spectrogram");
        layout1->addWidget(m_audio_vis_box, 2);
    }

    refresh_all();

    connect(
        m_audio_input_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (0 <= index && index < (int)m_input_sources.size()){
                m_input_sources[index]->run_post_select();
                m_session.set_audio_input(m_input_sources[index]);
            }else{
                m_session.set_audio_input(std::make_unique<AudioSourceDescriptor_Null>());
            }
        }
    );
    connect(
        m_audio_format_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index < 0 || index >= (int)m_input_choices.size()){
                return;
            }
            m_session.set_input_choice(m_input_choices[index]);
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
            m_session.spectrums().save_audio_frequencies_to_disk(m_record_is_on);
            if (m_record_is_on){
                m_record_button->setText("Stop recording");
            }else{
                m_record_button->setText("Record Frequencies");
            }
        });
    }

    session.add_state_listener(*this);
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
    refresh_input_device(m_session.input_source());
    refresh_formats(m_session.input_source(), m_session.input_choice());
    refresh_output_device(m_session.output_device());
    refresh_volume(m_session.output_volume());
    refresh_display(m_session.display_type());
}
void AudioSelectorWidget::refresh_formats(
    const std::shared_ptr<const AudioSourceDescriptor>& source,
    const AudioInputChoice& choice
){
    if (source == nullptr){
        m_audio_format_box->clear();
        return;
    }

    m_input_choices = source->supported_formats();

    m_audio_format_box->clear();
    int index = -1;
    for (size_t c = 0; c < m_input_choices.size(); c++){
        m_audio_format_box->addItem(QString::fromStdString(m_input_choices[c].display_name()));
        if (m_input_choices[c] == choice){
            index = (int)c;
        }
    }
    m_audio_format_box->setCurrentIndex(index);
}
void AudioSelectorWidget::refresh_input_device(const std::shared_ptr<const AudioSourceDescriptor>& source){
    std::shared_ptr<AudioSourceDescriptor> current_descriptor = m_session.input_source();

    std::vector<std::shared_ptr<AudioSourceDescriptor>> sources;

    //  Add all the static options.
    {
        AudioOption option;
        m_session.save(option);
        sources.emplace_back(option.get_input_from_cache(AudioSourceType::None));
        sources.emplace_back(option.get_input_from_cache(AudioSourceType::File));
    }

    //  Now add all the devices.
    for (AudioDeviceInfo& info : AudioDeviceInfo::all_input_devices()){
        sources.emplace_back(std::make_unique<AudioSourceDescriptor_Device>(std::move(info)));
    }

    m_input_sources = std::move(sources);
    m_audio_input_box->clear();

    int index = -1;
    for (int c = 0; c < (int)m_input_sources.size(); c++){
        const AudioSourceDescriptor& descriptor = *m_input_sources[c];
        m_audio_input_box->addItem(QString::fromStdString(descriptor.display_name()));
        if (*current_descriptor == descriptor){
            index = c;
        }
    }
    if (index >= 0){
        m_audio_input_box->setCurrentIndex((int)index);
    }else{
        m_session.logger().log("Unable to find entry for this source.", COLOR_RED);
    }
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


void AudioSelectorWidget::post_input_change(
    const std::shared_ptr<const AudioSourceDescriptor>& source,
    const AudioInputChoice& choice
){
//    cout << "AudioSelectorWidget::input_changed(): " << source.get() << endl;
    QMetaObject::invokeMethod(this, [this, source, choice]{
        refresh_input_device(source);
        refresh_formats(source, choice);
    }, Qt::QueuedConnection);   //  Queued due to potential recursive call to the same lock.
}
void AudioSelectorWidget::post_output_change(const AudioDeviceInfo& device){
    QMetaObject::invokeMethod(this, [this, device]{
        refresh_output_device(device);
    }, Qt::QueuedConnection);   //  Queued due to potential recursive call to the same lock.
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
