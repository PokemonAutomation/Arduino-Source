/*  FFT Streamer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_FFTStreamer_H
#define PokemonAutomation_AudioPipeline_FFTStreamer_H

#include <memory>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

class UnitTestDatabase;

namespace PokemonAutomation{



struct FFTListener{
    virtual void on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output) = 0;
};



//
//  The FFT endpoint of the pipeline tree. Consumes the tree's
//  single-channel working (float) stream, which the tree's mixer / resampler
//  nodes have already mixed and resampled to the FFT rate (48 kHz), and
//  produces FFTs for FFTListener children.
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
//  Registers the FFT gate tests (self-contained invariants on the
//  WorkingAudioToFFT node).
//
void add_tests_FFTStreamer(UnitTestDatabase& database);



}
#endif
