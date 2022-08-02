/*  Audio Display Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioDisplayWidget_H
#define PokemonAutomation_AudioPipeline_AudioDisplayWidget_H

#include <memory>
#include <deque>
#include <set>
#include <QWidget>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "AudioOption.h"
#include "AudioSpectrumHolder.h"

namespace PokemonAutomation{

class AudioThreadController;
class AudioDeviceInfo;
class LoggerQt;



// Display audio on the UI panel.
// This class will use AudioThreadController to control an audio thread doing the
// work of receiving audio streams from the capture card and playing it on the computer.
// The controller will also launch an FFT thread to do FFT computations on the audio
// stream and the resulting frequencies and the spectrogram can be visualized by this
// audio display widget class.
// The control of the audio, like which input and output audio device and which
// audio display mode to use, is made by AudioSelectorWidget.
// AudioSelectorWidget owns a reference of this class, and control this class based
// on the audio configuration the selector widget manages.
class AudioDisplayWidget : public QWidget, public AudioSpectrumHolder::Listener{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    using AudioDisplayType = AudioSelector::AudioDisplayType;

    AudioDisplayWidget(QWidget& parent);
    virtual ~AudioDisplayWidget();

    AudioSpectrumHolder& state(){ return m_spectrum_holder; }

    // outputVolume: range [0.f, 1.f]
    void set_audio(
        LoggerQt& logger,
        const AudioDeviceInfo& inputInfo,
        AudioFormat inputFormat,
        const std::string& inputAbsoluteFilepath,
        const AudioDeviceInfo& outputInfo,
        float outputVolume
    );

    void close_audio();

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    // Set audio display type: no display, frequency bars or spectrogram.
    void setAudioDisplayType(AudioDisplayType type);

public:
    // Below are implementation of some of functions in the `AudioFeed` interface.
    // AudioSelectWidget inherits AudioFeed, and calls the functions below to fulfill the AudioFeed interface.
    // See class `AudioFeed` for the comments of those functions.

    void add_overlay(size_t startingStamp, size_t endStamp, Color color);

signals:
    // Used to pass the changed volume from AudioSelectorWidget to audio thread.
    void volumeChanged(float volume);


private:
    virtual void audio_cleared() override;
    virtual void on_new_spectrum() override;
    virtual void on_new_overlay() override;

private:
    void update_size();
    void clear();

    void render_bars();
    void render_spectrograph();

private:
    AudioSpectrumHolder m_spectrum_holder;

    std::unique_ptr<AudioThreadController> m_audioThreadController;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;

    AudioDisplayType m_audioDisplayType = AudioDisplayType::NO_DISPLAY;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
