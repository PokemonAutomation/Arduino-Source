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
#include <list>
#include <fstream>
#include <QWidget>
#include <QThread>
#include <QAudioFormat>
#include "Common/Cpp/AlignedVector.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "AudioSelector.h"

class QString;

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
class AudioDisplayWidget : public QWidget{
    //  Need to define this Q_OBJECT to use Qt's extra features
    //  like signals and slots on this class.
    Q_OBJECT

public:
    using AudioDisplayType = AudioSelector::AudioDisplayType;

    AudioDisplayWidget(QWidget& parent);
    virtual ~AudioDisplayWidget();

    // outputVolume: range [0.f, 1.f]
    void set_audio(
        LoggerQt& logger,
        const AudioDeviceInfo& inputInfo,
        AudioFormat inputFormat,
        const QString& inputAbsoluteFilepath,
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

    std::vector<AudioSpectrum> spectrums_since(size_t startingStamp);

    std::vector<AudioSpectrum> spectrums_latest(size_t numLatestSpectrums);

    void add_overlay(size_t startingStamp, size_t endStamp, Color color);

public:
    // Development usage: save the FFT results to disk so that it can be examined
    // and edited to be used as samples for future audio matching.
    void saveAudioFrequenciesToDisk(bool enable);

signals:
    // Used to pass the changed volume from AudioSelectorWidget to audio thread.
    void volumeChanged(float volume);

public slots:
    // The audio thread (managed by m_audioThreadController) sends signal
    // to this slot to pass FFT outputs to the display.
    void loadFFTOutput(size_t sampleRate, std::shared_ptr<const AlignedVector<float>> fftOutput);

private:
    void update_size();

    void clear();

private:

    AudioThreadController* m_audioThreadController = nullptr;

    // Num frequencies to store for the output of one fft computation.
    const size_t m_numFreqs;
    // Num sliding fft windows to visualize.
    const size_t m_numFreqWindows;
    // Num blocks of frequencies to visualize for one sliding window.
    const size_t m_numFreqVisBlocks;
    // The boundaries to separate each frequency vis block.
    // i-th freq vis block is made by frequencies whose indices in m_spectrums
    // fall inside the range: [ m_freqVisBlockBoundaries[i], m_freqVisBlockBoundaries[i+1] )
    std::vector<size_t> m_freqVisBlockBoundaries;
    // Group nearby frequencies into blocks.
    // Each block uses the log scaled averaged magnitude of the frequencies.
    // stores those blocks together for visualization.
    std::vector<float> m_freqVisBlocks;
    // The timestamp of each window that's been visualized.
    std::vector<size_t> m_freqVisStamps;
    // The index of the next window in m_freqVisBlocks.
    size_t m_nextFFTWindowIndex = 0;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;

    AudioDisplayType m_audioDisplayType = AudioDisplayType::NO_DISPLAY;

    // record the past FFT output frequencies to serve as the interface
    // of audio inference for automation programs.
    // The head of the list is the most recent FFT window, while the tail
    // is the oldest in history.
    std::list<AudioSpectrum> m_spectrums;
    size_t m_spectrum_history_length = 10;
    // Since the spectrums will be probided to the automation programs in
    // other threads, we need to have a lock here.
    std::mutex m_spectrums_lock;

    // The inference boxes <box starting stamp, box end stamp, box color>
    // to highlight FFT windows on spectrogram. Used to tell user which part
    // of the audio is detected.
    // The head of the list is the most recent overlay added.
    std::list<std::tuple<size_t, size_t, Color>> m_overlay;
    // The overlay will be modified by the automation programs in other
    // threads to show inference boxes on the visualized spectrogram. So
    // we need a lock for it.
    std::mutex m_overlay_lock;

    // Develop purpose: used to save received frequencies to disk
    bool m_saveFreqToDisk = false;
    std::ofstream m_freqStream;
};




}
#endif
