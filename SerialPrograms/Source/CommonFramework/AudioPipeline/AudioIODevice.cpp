/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "AudioConstants.h"
#include "AudioIODevice.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



AudioIODevice::AudioIODevice(const QAudioFormat& audioFormat, AudioFormat format)
     : QIODevice(nullptr)
     , m_audioFormat(audioFormat)
     , m_format(format)
     , m_channelSwapBuffer(8192)
     , m_fft_input_buffer()
     , m_fftCircularBuffer(NUM_FFT_SAMPLES * 8)
//     , m_fftInputVector(NUM_FFT_SAMPLES)
{}

AudioIODevice::~AudioIODevice() {}


void AudioIODevice::write_output(const float* data, size_t samples){
    switch (m_format){
    case AudioFormat::MONO_48000:
    case AudioFormat::DUAL_44100:
    case AudioFormat::DUAL_48000:
    case AudioFormat::MONO_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        m_audioSinkDevice->write((const char*)data, samples * sizeof(float));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
    {
        // Interleaved mode: numChannels == 1 and sample rate should be 2 times the
        // normal sample rate. The input device interleaved L and R stereo channels into
        // one mono channel. The incoming samples follow the R - L order.
        // Since the stereo format has L - R order. We need to swap L and R samples:

        size_t out_samples = samples / 2;

        // First we need to make sure there are equal number of L and R samples to swap:
        if(samples % 2 != 0){
            std::cout << "Error: audio in interleaved mode but the number of samples "
                << samples << " is not even" << std::endl;
        }

        if (m_channelSwapBuffer.size() < samples){
            m_channelSwapBuffer.resize(samples);
        }

        //  Swapping samples:
        for(size_t i = 0; i < out_samples; i++){
            m_channelSwapBuffer[2*i + 0] = data[2*i + 1];
            m_channelSwapBuffer[2*i + 1] = data[2*i + 0];
        }
        m_audioSinkDevice->write((const char*)m_channelSwapBuffer.data(), samples * sizeof(float));

        break;
    }
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
}



qint64 AudioIODevice::writeData(const char* data, qint64 len)
{
//    const auto& audioFormat = m_audioFormat;
    // One audio frame consists of one or more channels.
    // Each channel in the frame has an audio sample that is made by one or
    // more bytes.
//    int frameBytes = audioFormat.bytesPerFrame();
//    int numChannels = audioFormat.channelCount();
//    int sampleBytes = frameBytes / numChannels;
    size_t numSamples = len / sizeof(float);
//    size_t numFrames = len / frameBytes;
    // We assert the data is always float with size of 4 bytes.
    const float* floatData = reinterpret_cast<const float *>(data);

//    static uint64_t c = 0;
//    cout << c++ << " - [0] = " << floatData[0] << ", [1] = " << floatData[1] << endl;

    // Pass audio data to the audio sink for audio playback.
    if (m_audioSinkDevice){
        write_output(floatData, numSamples);
    }



    //  Convert the audio input stream into a stream that our FFT can handle.
    size_t fft_sample_rate;
    size_t fft_samples;
    const float* fft_input;

    bool average_adjacent_samples = false;

    switch (m_format){
    case AudioFormat::MONO_48000:
        fft_sample_rate = 48000;
        fft_samples = len / sizeof(float);
        fft_input = floatData;
        average_adjacent_samples = false;
        break;
    case AudioFormat::DUAL_44100:
        fft_sample_rate = 44100;
        average_adjacent_samples = true;
        break;
    case AudioFormat::DUAL_48000:
    case AudioFormat::MONO_96000:
    case AudioFormat::INTERLEAVE_LR_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        fft_sample_rate = 48000;
        average_adjacent_samples = true;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }

    if (average_adjacent_samples){
        fft_samples = len / (sizeof(float) * 2);
        if (m_fft_input_buffer.size() < fft_samples){
            m_fft_input_buffer.resize(fft_samples);
        }
        fft_input = m_fft_input_buffer.data();
        for (size_t c = 0; c < fft_samples; c++){
            m_fft_input_buffer[c] = (floatData[2*c + 0] + floatData[2*c + 1]) * 0.5;
        }
    }



#define PASS_FFT
#ifdef PASS_FFT
    // numTotalFramesCopied: how many audio frames have been copied in the following loop
    size_t numTotalFramesCopied = 0;
    const size_t circularBufferSize = m_fftCircularBuffer.size();

    // while there are still samples not copied from audio source to the fft buffer:
    while (numTotalFramesCopied < fft_samples){

        // First, let's calculate how many more samples we need to file a FFT.
        size_t nextFFTNumFramesNeeded = computeNextFFTSamplesNeeded();

        // Copy as much data as possible from the audio source until either the input samples are
        // all used or the circular buffer reaches the end.
        size_t curNumFramesCopied = std::min(
            (size_t)m_fftCircularBuffer.size() - m_bufferNext,
            fft_samples - numTotalFramesCopied
        );

        memcpy(
            m_fftCircularBuffer.data() + m_bufferNext,
            fft_input + numTotalFramesCopied,
            sizeof(float) * curNumFramesCopied
        );

        // Update m_bufferNext to reflect new data added:
        m_bufferNext = (m_bufferNext + curNumFramesCopied) % circularBufferSize;
        // Update numTotalFramesCopied to reflect new data copied:
        numTotalFramesCopied += curNumFramesCopied;

        // With new data added, we need to check whether we can file one or more FFTs:

        size_t numNewDataForFFT = curNumFramesCopied;
        // While we have enough new data to file an FFT:
        while(nextFFTNumFramesNeeded <= numNewDataForFFT){
            // copy data from m_fftCircularBuffer to m_fftInputVector:
            std::shared_ptr<AlignedVector<float>> input_vector = std::make_unique<AlignedVector<float>>(moveDataToFFTInputVector());
//            std::cout << "FFT input copied " << std::endl;

            // emit signal for FFT:
            emit fftInputReady(fft_sample_rate, std::move(input_vector));

            // Update numNewDataForFFT to account for fft samples consumed:
            numNewDataForFFT -= nextFFTNumFramesNeeded;
            // Initialize nextFFTNumFramesNeeded for next FFT:
            nextFFTNumFramesNeeded = FFT_SLIDING_WINDOW_STEP;
            // Move m_fftStart forward for next FFT:
            m_fftStart = (m_fftStart + FFT_SLIDING_WINDOW_STEP) % circularBufferSize;
        }
    }
#endif


#ifdef DEBUG_AUDIO_IO
    std::vector<float> meanChannelVol(numChannels);
    std::vector<float> minChannelVol(numChannels, FLT_MAX);
    std::vector<float> maxChannelVol(numChannels, -FLT_MAX);
    for(int i = 0; i < numFrames; i++){
        for(int j = 0; j < numChannels; j++){
            const float v = floatData[i*numChannels+j];
            meanChannelVol[j] += v;
            minChannelVol[j] = std::min(minChannelVol[j], v);
            maxChannelVol[j] = std::max(maxChannelVol[j], v);
        }
    }
    for(int j = 0; j < numChannels; j++){
        meanChannelVol[j] /= float(numSamples);
    }
    auto currentTimepoint = std::chrono::system_clock::now();
    const auto dur = std::chrono::duration_cast<std::chrono::microseconds>(currentTimepoint-m_lastWriteTimepoint);
    m_lastWriteTimepoint = currentTimepoint;
    std::cout << "T" << QThread::currentThread() << " Wrote " << numSamples << " audio frames after " <<
        dur.count() << " mircoseconds, mean/min/max volume per channel: [";
    for(int j = 0; j < audioFormat.channelCount(); j++){
        std::cout << std::setprecision(3) << meanChannelVol[j] << "/" <<
            std::setprecision(3) << minChannelVol[j] << "/" <<
            std::setprecision(3) << maxChannelVol[j] << ", ";
    }
    std::cout << "]" << std::endl;
#endif
    return len;
}

size_t AudioIODevice::computeNextFFTSamplesNeeded() const{
    // nextFFTNumSamplesFilled: how many samples already in the buffer to be used for the next FFT:
    size_t nextFFTNumSamplesFilled = 0;
    if (m_bufferNext >= m_fftStart){
        nextFFTNumSamplesFilled = m_bufferNext - m_fftStart;
    }else{
        nextFFTNumSamplesFilled = m_bufferNext + m_fftCircularBuffer.size() - m_fftStart;
    }
    if (nextFFTNumSamplesFilled >= NUM_FFT_SAMPLES){
        std::cout << "ERROR: internal buffer error, stored more than enough FFT samples but did not launch FFT "
            << nextFFTNumSamplesFilled << " " << m_bufferNext << " " << m_fftStart << std::endl;
    }
    assert(nextFFTNumSamplesFilled < NUM_FFT_SAMPLES);

    return NUM_FFT_SAMPLES - nextFFTNumSamplesFilled;
}

AlignedVector<float> AudioIODevice::moveDataToFFTInputVector(){
    AlignedVector<float> out(NUM_FFT_SAMPLES);
    size_t num = std::min(m_fftCircularBuffer.size() - m_fftStart, (size_t)NUM_FFT_SAMPLES);
//    memcpy(m_fftInputVector.data(), m_fftCircularBuffer.data() + m_fftStart, sizeof(float) * num);
    memcpy(out.data(), m_fftCircularBuffer.data() + m_fftStart, sizeof(float) * num);
    if (num < NUM_FFT_SAMPLES){
        // We reach the end of the circular buffer, start from its beginning to copy the remaining part:
//        memcpy(m_fftInputVector.data() + num, m_fftCircularBuffer.data(), sizeof(float) * (NUM_FFT_SAMPLES - num));
        memcpy(out.data() + num, m_fftCircularBuffer.data(), sizeof(float) * (NUM_FFT_SAMPLES - num));
    }
    return out;
}



}
