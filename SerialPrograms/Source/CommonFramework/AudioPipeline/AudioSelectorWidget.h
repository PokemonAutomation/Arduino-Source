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

class AudioDisplayWidget;

//  Widget to handle UI that selects audio source.
//  The audio state is managed by a class AudioSelector object passed in
//  the constructor.
//  AudioSelectorWidget is also responsible for updating AudioDisplayWidget,
//  the UI that visualizes audio, when the audio source is changed. The
//  AudioDisplayWidget is passed in the constructor too.
class AudioSelectorWidget : public QWidget, public AudioFeed{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT
public:
    AudioSelectorWidget(
        QWidget& parent,
        LoggerQt& logger,
        AudioSelector& value,
        AudioDisplayWidget& holder
    );
    ~AudioSelectorWidget();

    void set_audio_enabled(bool enabled);

    void reset_audio();

    // Functions below are implementations of the interfaces in `AudioFeed`.
    // See class `AudioFeed` for the comments of those functions.

    virtual void reset() override;

    virtual std::vector<AudioSpectrum> spectrums_since(uint64_t startingStamp) override;

    virtual std::vector<AudioSpectrum> spectrums_latest(size_t numLatestSpectrums) override;

    virtual void add_overlay(uint64_t startingStamp, size_t endStamp, Color color) override;

signals:
    // Need to define this version of async_reset_audio()
    // because it will be used as a signal.
    void internal_async_reset_audio();

private:
    void update_formats();
    void refresh();

private:
    LoggerQt& m_logger;
    AudioSelector& m_value;

    AudioDisplayWidget& m_display;

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
