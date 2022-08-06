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
#include "AudioSession.h"
#include "AudioDisplayWidget.h"
#include "AudioSelectorWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



AudioSelectorWidget::~AudioSelectorWidget(){}

AudioSelectorWidget::AudioSelectorWidget(
    QWidget& parent,
    LoggerQt& logger,
    AudioSession& session
)
    : QWidget(&parent)
    , m_logger(logger)
    , m_session(session)
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
        m_audio_input_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            if (index <= 0 || index > (int)m_input_audios.size()){
                m_session.clear_audio_input();
            }else{
                m_session.set_audio_input(m_input_audios[index - 1]);
            }
            update_formats();
        }
    );
    connect(
        m_audio_format_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            const std::vector<AudioChannelFormat>& supported_formats = m_session.option().input_device().supported_formats();
            if (index < 0 || index >= (int)supported_formats.size()){
                return;
            }
            m_session.set_format(supported_formats[index]);
        }
    );
    connect(
        m_audio_output_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            if (index <= 0 || index > (int)m_output_audios.size()){
                m_session.clear_audio_output();
            }else{
                m_session.set_audio_output(m_output_audios[index - 1]);
            }
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
            refresh();
            m_session.reset();
            update_formats();

        }
    );
    connect(
        m_audio_vis_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0 || index > 2){
                index = 0;
            }
            m_session.set_display((AudioOption::AudioDisplayType)index);
        }
    );

    connect(
        m_volume_slider, &QSlider::valueChanged, this, [=](){
            m_session.set_volume(m_volume_slider->value() / 100.);
        }
    );

    // only in developer mode:
    // record audio
    if (PreloadSettings::instance().DEVELOPER_MODE){
        connect(m_record_button, &QPushButton::clicked, this, [=](bool){
            m_record_is_on = !m_record_is_on;
            m_session.spectrums().saveAudioFrequenciesToDisk(m_record_is_on);
            if (m_record_is_on){
                m_record_button->setText("Stop recording");
            } else{
                m_record_button->setText("Record Frequencies");
            }
        });

        connect(m_load_file_button,  &QPushButton::clicked, this, [=](bool){
            std::string path = QFileDialog::getOpenFileName(this, tr("Open audio file"), ".", "*.wav *.mp3").toStdString();
            if (path.empty()){
                return;
            }
            m_session.set_audio_input(std::move(path));
            m_audio_input_box->setCurrentIndex(0);
        });
    }
}
void AudioSelectorWidget::update_formats(){
    m_audio_format_box->clear();

    if (!m_session.option().input_file().empty()){
        return;
    }

    const AudioDeviceInfo& device = m_session.option().input_device();
    AudioChannelFormat current_format = m_session.option().input_format();

    const std::vector<AudioChannelFormat>& supported_formats = device.supported_formats();

    int index = -1;
    for (size_t c = 0; c < supported_formats.size(); c++){
        m_audio_format_box->addItem(AUDIO_FORMAT_LABELS[(size_t)supported_formats[c]]);
        if (current_format == supported_formats[c]){
            index = (int)c;
        }
    }
    m_audio_format_box->setCurrentIndex(index);
}

void AudioSelectorWidget::refresh(){

    auto build_device_menu = [](
        QComboBox* box,
        const std::vector<AudioDeviceInfo>& list,
        const AudioDeviceInfo& current_device
    ){
        
        box->clear();
        box->addItem("(none)");

        size_t index = 0;
        for (size_t c = 0; c < list.size(); c++){
            const AudioDeviceInfo& audio = list[c];
            box->addItem(QString::fromStdString(audio.display_name()));

            if (current_device == audio){
                index = c + 1;
            }
        }
        if (index != 0){
            box->setCurrentIndex((int)index);
            return true;
        }else{
            box->setCurrentIndex(0);
            return false;
        }
    };


    m_input_audios = AudioDeviceInfo::all_input_devices();
    m_output_audios = AudioDeviceInfo::all_output_devices();
    if (!build_device_menu(m_audio_input_box, m_input_audios, m_session.option().input_device())){
        m_session.clear_audio_input();
    }
    if (!build_device_menu(m_audio_output_box, m_output_audios, m_session.option().output_device())){
        m_session.clear_audio_output();
    }

    update_formats();

    // std::cout << "Refresh: " << AudioSelector::audioDisplayTypeToString(m_value.m_audioDisplayType) << std::endl;
    switch(m_session.option().display_type()){
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
//    m_session.set_display(m_value.m_audioDisplayType);

    m_volume_slider->setValue((int)(m_session.option().volume() * 100));
}





}
