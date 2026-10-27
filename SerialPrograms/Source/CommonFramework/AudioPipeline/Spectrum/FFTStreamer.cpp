/*  FFT Streamer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/KaiserWindow/Kernels_KaiserWindow.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "FFTStreamer.h"

namespace PokemonAutomation{



void WorkingAudioToFFT::add_listener(FFTListener& listener){
    m_listeners.add(listener);
}
void WorkingAudioToFFT::remove_listener(FFTListener& listener){
    m_listeners.remove(listener);
}

WorkingAudioToFFT::WorkingAudioToFFT(size_t sample_rate)
    : WorkingAudioStreamListener(sample_rate, 1)
    , m_sample_rate(sample_rate)
    , m_buffer(NUM_FFT_SAMPLES)
    , m_buffered(NUM_FFT_SAMPLES)
    , m_fft_input(NUM_FFT_SAMPLES)
{
    memset(m_buffer.data(), 0, m_buffer.size() * sizeof(float));
}
WorkingAudioToFFT::~WorkingAudioToFFT(){}
void WorkingAudioToFFT::push_samples(size_t samples, size_t stride, const float* data){
    //  The tree delivers a single-channel working stream; channel 0 is
    //  contiguous regardless of stride (which may be 0 on the
    //  zero-copy single-channel float path).
    const float* ptr = data;
    while (samples > 0){
        //  Figure out how much space we can write contiguously.
        size_t block = m_buffer.size() - std::max(m_buffered, m_end);

        //  Don't write more than we have.
        block = std::min(block, samples);

        memcpy(&m_buffer[m_end], ptr, block * sizeof(float));
        m_buffered += block;
        m_end += block;
        if (m_end == m_buffer.size()){
            m_end = 0;
        }
        ptr += block;
        samples -= block;

        //  Buffer is full. Time to run FFT!
        if (m_buffered == m_buffer.size()){
            run_fft();
            drop_from_front(FFT_SLIDING_WINDOW_STEP);
        }
    }
}
void WorkingAudioToFFT::run_fft(){
    float* ptr = m_fft_input.data();
    size_t remaining = NUM_FFT_SAMPLES;
    size_t index = m_start;
    while (remaining > 0){
        size_t block = std::min(remaining, m_buffer.size() - index);
        memcpy(ptr, &m_buffer[index], block * sizeof(float));
        ptr += block;
        remaining -= block;
        index += block;
        if (index == m_buffer.size()){
            index = 0;
        }
    }
    std::shared_ptr<AlignedVector<float>> out = std::make_unique<AlignedVector<float>>(NUM_FFT_SAMPLES / 2);
    Kernels::AbsFFT::fft_abs(
        FFT_LENGTH_POWER_OF_TWO,
        out->data(),
        m_fft_input.data(),
        GlobalSettings::instance().AUDIO_PIPELINE->USE_KAISER_WINDOW
            ? Kernels::KaiserWindow::get_kaiser_window_k(FFT_LENGTH_POWER_OF_TWO)
            : nullptr
    );
    m_listeners.run_method(
        &FFTListener::on_fft,
        m_sample_rate, out
    );
}
void WorkingAudioToFFT::drop_from_front(size_t frames){
    if (frames >= m_buffered){
        m_buffered = 0;
        m_start = 0;
        m_end = 0;
        return;
    }
    m_buffered -= frames;
    m_start += frames;
    while (m_start >= m_buffer.size()){
        m_start -= m_buffer.size();
    }
}




}
