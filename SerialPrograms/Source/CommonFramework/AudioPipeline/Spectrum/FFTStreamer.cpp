/*  FFT Streamer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "FFTStreamer.h"

namespace PokemonAutomation{



std::unique_ptr<AudioFloatToFFT> make_FFT_streamer(AudioChannelFormat format){
    switch (format){
    case AudioChannelFormat::MONO_48000:
        return std::make_unique<AudioFloatToFFT>(48000, 1, false);
    case AudioChannelFormat::DUAL_44100:
        return std::make_unique<AudioFloatToFFT>(44100, 2, true);
    case AudioChannelFormat::DUAL_48000:
    //  Treat mono-96000 as 2-sample frames.
    //  The FFT will then average each pair to produce 48000Hz.
    //  The output will push the same stream at the original 4 bytes * 96000Hz.
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        return std::make_unique<AudioFloatToFFT>(48000, 2, true);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }
}



void AudioFloatToFFT::add_listener(FFTListener& listener){
    m_listeners.add(listener);
}
void AudioFloatToFFT::remove_listener(FFTListener& listener){
    m_listeners.remove(listener);
}

AudioFloatToFFT::AudioFloatToFFT(
    size_t sample_rate,
    size_t samples_per_frame, bool average_pairs
)
    : AudioFloatStreamListener(samples_per_frame)
    , m_sample_rate(sample_rate)
    , m_average(average_pairs)
    , m_fft_sample_size(average_pairs ? 2 : 1)
    , m_buffer(NUM_FFT_SAMPLES)
    , m_buffered(NUM_FFT_SAMPLES)
    , m_fft_input(NUM_FFT_SAMPLES)
{
    if (samples_per_frame == 0 || samples_per_frame > 2){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channels must be 1 or 2.");
    }
    memset(m_buffer.data(), 0, m_buffer.size() * sizeof(float));
}
AudioFloatToFFT::~AudioFloatToFFT(){}
void AudioFloatToFFT::on_samples(const float* data, size_t frames){
//    cout << "objects = " << objects << endl;
    const float* ptr = data;
    while (frames > 0){
        //  Figure out how much space we can write contiguously.
        size_t block = m_buffer.size() - std::max(m_buffered, m_end);

        //  Don't write more than we have.
        block = std::min(block, frames);

//        cout << "block = " << block << endl;

        //  Write it.
        convert(&m_buffer[m_end], ptr, block);
        m_buffered += block;
        m_end += block;
        if (m_end == m_buffer.size()){
            m_end = 0;
        }
        ptr += block * m_fft_sample_size;
        frames -= block;

        //  Buffer is full. Time to run FFT!
        if (m_buffered == m_buffer.size()){
//            cout << "run_fft()" << endl;
            run_fft();
            drop_from_front(FFT_SLIDING_WINDOW_STEP);
        }
    }
}
void AudioFloatToFFT::convert(float* fft_input, const float* audio_stream, size_t frames){
    if (!m_average){
        memcpy(fft_input, audio_stream, frames * sizeof(float));
        return;
    }
    for (size_t c = 0; c < frames; c++){
        fft_input[c] = (audio_stream[2*c + 0] + audio_stream[2*c + 1]) * 0.5f;
    }
}
void AudioFloatToFFT::run_fft(){
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
    Kernels::AbsFFT::fft_abs(FFT_LENGTH_POWER_OF_TWO, out->data(), m_fft_input.data());
    m_listeners.run_method_unique(
        &FFTListener::on_fft,
        m_sample_rate, out
    );
}
void AudioFloatToFFT::drop_from_front(size_t frames){
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
