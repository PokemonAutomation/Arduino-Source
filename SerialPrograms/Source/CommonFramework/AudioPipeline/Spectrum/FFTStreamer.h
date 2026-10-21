/*  FFT Streamer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_FFTStreamer_H
#define PokemonAutomation_AudioPipeline_FFTStreamer_H

#include <memory>
#include "Common/Cpp/ListenerSet.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

class UnitTestDatabase;

namespace PokemonAutomation{



struct FFTListener{
    virtual void on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output) = 0;
};



//  Listen to an audio stream and compute FFTs on it.
class AudioFloatToFFT : public AudioFloatStreamListener{
public:
    void add_listener(FFTListener& listener);
    void remove_listener(FFTListener& listener);

public:
    AudioFloatToFFT(
        size_t sample_rate,
        size_t samples_per_frame, bool average_pairs
    );
    virtual ~AudioFloatToFFT();
    virtual void on_samples(const float* data, size_t frames) override;

private:
    void convert(float* fft_input, const float* audio_stream, size_t frames);
    void run_fft();
    void drop_from_front(size_t frames);

private:
    size_t m_sample_rate;

    bool m_average;
    size_t m_fft_sample_size;

    AlignedVector<float> m_buffer;
    size_t m_buffered = 0;
    size_t m_start = 0;
    size_t m_end = 0;

    AlignedVector<float> m_fft_input;

    ListenerSet<FFTListener> m_listeners;
};


std::unique_ptr<AudioFloatToFFT> make_FFT_streamer(AudioChannelFormat format);



//
//  The FFT endpoint of the new pipeline tree. Consumes the tree's
//  single-channel working (float) stream, which the tree's mixer / resampler
//  nodes have already mixed and resampled to the FFT rate (48 kHz), and
//  produces FFTs for FFTListener children. It is the WorkingAudioStreamListener
//  counterpart of AudioFloatToFFT (which consumes the legacy interleaved
//  float stream): on the no-op 48 kHz case the two paths are bit-identical
//  (see the gate tests registered by add_tests_FFTStreamer).
//
class WorkingAudioToFFT : public Kernels::AudioPipeline::WorkingAudioStreamListener{
public:
    void add_listener(FFTListener& listener);
    void remove_listener(FFTListener& listener);

public:
    explicit WorkingAudioToFFT(size_t sample_rate);
    virtual ~WorkingAudioToFFT();
    virtual void push_samples(size_t samples, size_t stride, const float* data) override;

private:
    void run_fft();
    void drop_from_front(size_t frames);

private:
    const size_t m_sample_rate;

    AlignedVector<float> m_buffer;
    size_t m_buffered = 0;
    size_t m_start = 0;
    size_t m_end = 0;

    AlignedVector<float> m_fft_input;

    ListenerSet<FFTListener> m_listeners;
};



//
//  Registers the FFT migration gate tests (tree FFT node vs the legacy
//  AudioStreamToFloat -> AudioFloatToFFT path).
//
void add_tests_FFTStreamer(UnitTestDatabase& database);



}
#endif
