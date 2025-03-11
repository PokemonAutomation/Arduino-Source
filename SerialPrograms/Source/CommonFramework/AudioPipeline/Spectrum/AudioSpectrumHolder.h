/*  Audio Spectrum Holder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSpectrumHolder_H
#define PokemonAutomation_AudioPipeline_AudioSpectrumHolder_H

#include <list>
#include <set>
#include <mutex>
#include <fstream>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/ListenerSet.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "Spectrograph.h"

namespace PokemonAutomation{


class AudioSpectrumHolder{
public:
    struct Listener{
        virtual void state_changed() = 0;
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    AudioSpectrumHolder();

    void clear();
//    virtual void reset() override;


public:
    void push_spectrum(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output);
    void add_overlay(uint64_t starting_stamp, uint64_t end_stamp, Color color);


public:
    //  Asynchronous and thread-safe getters.

    std::vector<AudioSpectrum> spectrums_since(uint64_t starting_stamp);
    std::vector<AudioSpectrum> spectrums_latest(size_t num_latest_spectrums);

    struct SpectrumSnapshot{
        WallClock timestamp;
        std::vector<float> values;
        std::vector<uint32_t> colors;
    };
    SpectrumSnapshot get_last_spectrum() const;

    struct SpectrographSnapshot{
        ImageRGB32 image;
        std::vector<std::tuple<size_t, size_t, Color>> overlays;
    };
    SpectrographSnapshot get_spectrograph() const;


public:
    // Development usage: save the FFT results to disk so that it can be examined
    // and edited to be used as samples for future audio matching.
    void saveAudioFrequenciesToDisk(bool enable);


private:
    // Num frequencies to store for the output of one fft computation.
    const size_t m_num_freqs;
    // Num sliding fft windows to visualize.
    const size_t m_num_freq_windows;
    // Num blocks of frequencies to visualize for one sliding window.
//    const size_t m_num_freq_visualization_blocks;

    // The boundaries to separate each frequency vis block.
    // i-th freq vis block is made by frequencies whose indices in m_spectrums
    // fall inside the range: [ m_freq_visualization_block_boundaries[i], m_freq_visualization_block_boundaries[i+1] )
    std::vector<size_t> m_freq_visualization_block_boundaries;

    SpectrumSnapshot m_last_spectrum;
    std::unique_ptr<Spectrograph> m_spectrograph;

    // The timestamp of each window that's been visualized.
    std::vector<uint64_t> m_freqVisStamps;
    // The index of the next window in m_freqVisBlocks.
    size_t m_nextFFTWindowIndex = 0;

    // record the past FFT output frequencies to serve as the interface
    // of audio inference for automation programs.
    // The head of the list is the most recent FFT window, while the tail
    // is the oldest in history.
    std::list<AudioSpectrum> m_spectrums;
    size_t m_spectrum_history_length = 40;
    // The initial timestamp for the incoming spectrums.
    size_t m_spectrum_stamp_start = 0;

    // Develop purpose: used to save received frequencies to disk
    bool m_saveFreqToDisk = false;
    std::ofstream m_freqStream;

    // The inference boxes <box starting stamp, box end stamp, box color>
    // to highlight FFT windows on spectrogram. Used to tell user which part
    // of the audio is detected.
    // The head of the list is the most recent overlay added.
    std::list<std::tuple<size_t, size_t, Color>> m_overlay;

    mutable std::mutex m_state_lock;
    ListenerSet<Listener> m_listeners;
};



}
#endif
